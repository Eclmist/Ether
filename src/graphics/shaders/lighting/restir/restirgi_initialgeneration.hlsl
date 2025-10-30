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

[shader("raygeneration")]
void RayGeneration()
{
    const uint2 sampleCoords = DispatchRaysIndex().xy;
    const uint2 screenCoords = GetScreenCoordsFromSampleCoords(sampleCoords);
    const uint2 bufferSize = DispatchRaysDimensions().xy;
    const uint sampleIdx = GetSampleIndexFromSampleCoords(sampleCoords, bufferSize);

    const ShadingSurface surface = GetShadingSurfaceFromGBuffers(screenCoords, g_GBufferA, g_GBufferB, g_GBufferC, g_GBufferD);
    const float3 viewDir = normalize(g_GlobalConstants.m_CameraPosition.xyz - surface.m_Position);
    const float3 wo = normalize(g_GlobalConstants.m_CameraPosition.xyz - surface.m_Position);

    float3 wi;
    float pdf;

#if USE_IMPORTANCE_SAMPLING
    SampleDirectionBrdf(surface, g_GlobalConstants.m_FrameNumber, viewDir, wi, pdf);
#else
    SampleDirectionUniform(surface, g_GlobalConstants.m_FrameNumber, wi, pdf);
#endif

    const RayPayload payload = TraceShadingRay(surface, wi, MAX_DEPTH);
    GIReservoirSample initialSample = GIReservoirSample::Empty();
    initialSample.m_Position = payload.m_HitPosition;
    initialSample.m_Normal = payload.m_HitNormal;
    initialSample.m_Radiance = payload.m_Radiance;

    const float3 targetFunction = ComputeTargetFunction(surface, initialSample);
    const float3 risWeight = targetFunction / pdf;

    GIReservoir initialReservoir = GIReservoir::Empty();
    initialReservoir.Resample(initialSample, Random(screenCoords, g_GlobalConstants.m_FrameNumber), targetFunction, risWeight);
    g_RWOutputReservoir[sampleIdx] = GIReservoir::Pack(initialReservoir);
}

[shader("closesthit")]
void ClosestHit(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attribs)
{
    const GeometryInfo geoInfo = g_GeometryInfo[InstanceIndex()];
    const MeshVertex vertex = GetHitSurface(attribs, geoInfo);
    const Material material = g_MaterialTable[geoInfo.m_MaterialIndex];

    ShadingSurface surface = GetShadingSurfaceFromHit(vertex, material, g_GlobalConstants.m_SamplerIndex_Linear_Wrap, INDIRECT_MIP_LEVEL);

    payload.m_Hit = true;
    payload.m_HitPosition = surface.m_Position;
    payload.m_HitNormal = surface.m_Normal;
    payload.m_Depth = max(0, (int) payload.m_Depth - 1);
    payload.m_Radiance = 0;

    if (payload.m_Depth <= 0)
        return;

    if (payload.m_IsShadowRay)
        return;

    const RayPayload shadowRay = TraceShadowRay(surface, g_GlobalConstants.m_SunDirection.xyz);
    const float3 wo = normalize(-WorldRayDirection());
    const float3 wi = normalize(g_GlobalConstants.m_SunDirection.xyz);
    payload.m_Radiance = surface.m_Emission + ComputeRadiance(surface, shadowRay.m_Radiance, wi, wo);
}
