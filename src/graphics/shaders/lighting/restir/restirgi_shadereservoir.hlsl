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

RWTexture2D<float4> g_LightingOutput                        : register(u3);

void SampleDirectionBrdf(ShadingSurface surface, out float3 wi, out float pdf)
{
    const uint3 launchIndex = DispatchRaysIndex();
    const uint3 launchDim = DispatchRaysDimensions();
    const uint sampleIdx = launchIndex.y * launchDim.x + launchIndex.x;
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

[shader("raygeneration")]
void RayGeneration()
{
    const uint2 screenCoords = DispatchRaysIndex().xy;
    const uint2 screenSize = DispatchRaysDimensions().xy;
    uint sampleIdx = GetSampleIndexFromScreenCoords(screenCoords, screenSize);

#if DOWNSAMPLE_FACTOR != 1
    const float2 stochasticOffsets = (CMJ_Sample2D(sampleIdx, 1024, 1024, g_GlobalConstants.m_FrameNumber + 400.0f) - 0.5f) * 2.0f;
    const uint2 sampleCoords = round( GetSampleCoordsFromScreenCoords(screenCoords) + stochasticOffsets);
    sampleIdx = GetSampleIndexFromSampleCoords(sampleCoords, screenSize / (float)DOWNSAMPLE_FACTOR);
#endif

    const ShadingSurface diffuseSurface = GetShadingSurfaceFromGBuffers(screenCoords, g_GBufferA, g_GBufferB, g_GBufferC, g_GBufferD);
    const ShadingSurface surface = GetShadingSurfaceFromGBuffers(screenCoords, g_GBufferA, g_GBufferB, g_GBufferC, g_GBufferD, false);
    GIReservoir finalReservoir = GIReservoir::Unpack(g_InputReservoir[sampleIdx]);

    const RayPayload shadowRay = TraceShadowRay(surface);
    const float3 Li = shadowRay.m_Radiance;
    const float3 wo = normalize(g_GlobalConstants.m_CameraPosition.xyz - surface.m_Position);
    const float3 wi = normalize(g_GlobalConstants.m_SunDirection.xyz);
    const float3 f = BRDF_UE4(wi, wo, surface.m_Normal, surface.m_Albedo, surface.m_Roughness, surface.m_Metalness);
    const float cosTheta = saturate(dot(wi, surface.m_Normal));
    const float3 directLighting = f * Li * cosTheta;
    const RayPayload validationRay = TraceValidationRay(diffuseSurface, finalReservoir.m_Sample);
    
    if (validationRay.m_Hit)
        finalReservoir = GIReservoir::Empty();

    finalReservoir.FinalizeResampling();
    g_LightingOutput[screenCoords].xyz = directLighting + ComputeRadiance(diffuseSurface, finalReservoir.m_Sample) * finalReservoir.m_WeightSum;
    g_LightingOutput[screenCoords].a = 0;

}

[shader("miss")]
void Miss(inout RayPayload payload)
{
    payload.m_Hit = false;

    // Sample sun color
    const float lerpFactor = saturate(dot(g_GlobalConstants.m_SunDirection.xyz, float3(0, 1, 0)));
    payload.m_Radiance = lerp(0.0f, g_GlobalConstants.m_SunColor.xyz, lerpFactor);

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

    if (payload.m_Depth <= 0)
    {
        payload.m_Radiance = surface.m_Emission;
    }
    else
    {
        const RayPayload shadowRay = TraceShadowRay(surface);
        const float3 wo = normalize(-WorldRayDirection());
        const float3 wi = normalize(g_GlobalConstants.m_SunDirection.xyz);
        payload.m_Radiance = ComputeRadiance(surface, shadowRay.m_Radiance, wi, wo);
    }
}
