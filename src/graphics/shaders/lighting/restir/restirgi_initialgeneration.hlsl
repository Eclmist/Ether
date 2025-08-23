/*
    This file is part of Ether, an open-source DirectX 12 renderer.

    Copyright (c) 2020-2023 Samuel Huang - All rights reserved.

    Ether is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "lighting/restir/gireservoirresampling.hlsl"

#define USE_IMPORTANCE_SAMPLING 1

void SampleDirectionBrdf(ShadingSurface surface, out float3 wi, out float pdf)
{
    const uint2 sampleCoords = DispatchRaysIndex().xy;
    const uint2 bufferSize = DispatchRaysDimensions().xy;
    const uint sampleIdx = GetSampleIndexFromSampleCoords(sampleCoords, bufferSize);
    const float2 rand2D = CMJ_Sample2D(sampleIdx, 1024, 1024, g_GlobalConstants.m_FrameNumber);

    const float3 wo = normalize(g_GlobalConstants.m_CameraPosition.xyz - surface.m_Position);
    const float diffuseWeight = lerp(lerp(0.5, 1.0, surface.m_Roughness), 0.0, surface.m_Metalness);
    const float specularWeight = 1.0 - diffuseWeight;

    const bool importanceSampleBrdf = Random(rand2D.x) <= specularWeight;

    if (importanceSampleBrdf)
        wi = normalize(ImportanceSampleGGX(rand2D, wo, surface.m_Normal, surface.m_Roughness));
    else
        wi = normalize(TangentToWorld(SampleDirectionCosineHemisphere(rand2D), surface.m_Normal));

    const float3 H = normalize(wi + wo);
    const float nDotH = saturate(dot(surface.m_Normal, H));
    const float nDotV = saturate(dot(surface.m_Normal, wo));
    const float vDotH = saturate(dot(wo, H));

    // TODO: Abs is also wrong here. Why does it work?
    const float cosTheta = abs(dot(-wi, surface.m_Normal));

    if (importanceSampleBrdf)
        pdf = UE4JointPdf(specularWeight, nDotH, cosTheta, vDotH, surface.m_Roughness);
    else
        pdf = SampleDirectionHemisphere_Pdf();
}

void SampleDirectionUniform(ShadingSurface surface, out float3 wi, out float pdf)
{
    const uint2 sampleCoords = DispatchRaysIndex().xy;
    const uint2 bufferSize = DispatchRaysDimensions().xy;
    const uint sampleIdx = GetSampleIndexFromSampleCoords(sampleCoords, bufferSize);
    const float2 rand2D = CMJ_Sample2D(sampleIdx, 1024, 1024, g_GlobalConstants.m_FrameNumber);
    wi = TangentToWorld(SampleDirectionHemisphere(rand2D), surface.m_Normal);
    pdf = SampleDirectionHemisphere_Pdf();
}

[shader("raygeneration")]
void RayGeneration()
{
    const uint2 sampleCoords = DispatchRaysIndex().xy;
    const uint2 screenCoords = GetScreenCoordsFromSampleCoords(sampleCoords);
    const uint2 bufferSize = DispatchRaysDimensions().xy;
    const uint sampleIdx = GetSampleIndexFromSampleCoords(sampleCoords, bufferSize);

    ShadingSurface surface = GetShadingSurfaceFromGBuffers(screenCoords, g_GBufferA, g_GBufferB, g_GBufferC, g_GBufferD);
    const float3 wo = normalize(g_GlobalConstants.m_CameraPosition.xyz - surface.m_Position);

    //surface.m_Normal = dot(wo, surface.m_Normal) < 0 ? -surface.m_Normal : surface.m_Normal;

    float3 wi;
    float pdf;

#if USE_IMPORTANCE_SAMPLING
    SampleDirectionBrdf(surface, wi, pdf);
#else
    SampleDirectionUniform(surface, wi, pdf);
#endif

    const RayPayload payload = TraceShadingRay(surface.m_Position, wi, MAX_DEPTH);
    GIReservoirSample initialSample = GIReservoirSample::Empty();
    initialSample.m_Position = payload.m_HitPosition;
    initialSample.m_Normal = payload.m_HitNormal;
    initialSample.m_Radiance = payload.m_Radiance;

    const float3 targetFunction = ComputeRadiance(surface, initialSample);
    const float3 risWeight = targetFunction / pdf;

    GIReservoir initialReservoir = GIReservoir::Empty();
    initialReservoir.Resample(initialSample, Random(screenCoords, g_GlobalConstants.m_FrameNumber), targetFunction, risWeight);
    g_RWOutputReservoir[sampleIdx] = GIReservoir::Pack(initialReservoir);
}

[shader("miss")]
void Miss(inout RayPayload payload)
{
    payload.m_Hit = false;
    payload.m_Radiance = 0;
    payload.m_HitPosition = WorldRayOrigin() + WorldRayDirection() * 9999.0f;

    if (payload.m_IsShadowRay)
    {
        // Sample sun color
        const float lerpFactor = saturate(dot(g_GlobalConstants.m_SunDirection.xyz, float3(0, 1, 0)));
        payload.m_Radiance = lerp(0.0f, g_GlobalConstants.m_SunColor.xyz, lerpFactor);
    }
    else
    {
        // Sample environment color
        payload.m_Radiance = SampleEnvironmentLighting(WorldRayDirection());
    }
}

[shader("closesthit")]
void ClosestHit(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attribs)
{
    const GeometryInfo geoInfo = g_GeometryInfo[InstanceIndex()];
    const MeshVertex vertex = GetHitSurface(attribs, geoInfo);
    const Material material = g_MaterialTable[geoInfo.m_MaterialIndex];
    const ShadingSurface surface = GetShadingSurfaceFromHit(vertex, material, g_GlobalConstants.m_SamplerIndex_Linear_Wrap, INDIRECT_MIP_LEVEL);

    payload.m_Hit = true;
    payload.m_HitPosition = surface.m_Position;
    payload.m_HitNormal = surface.m_Normal;
    payload.m_Depth = max(0, (int)payload.m_Depth - 1);
    payload.m_Radiance = 0;

    if (payload.m_IsShadowRay)
        return;

    const RayPayload shadowRay = TraceShadowRay(surface);
    const float3 wo = normalize(-WorldRayDirection());
    const float3 wi = normalize(g_GlobalConstants.m_SunDirection.xyz);
    payload.m_Radiance = surface.m_Emission + ComputeRadiance(surface, shadowRay.m_Radiance, wi, wo);
}
