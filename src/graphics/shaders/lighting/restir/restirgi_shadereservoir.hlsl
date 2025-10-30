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

[shader("raygeneration")]
void RayGeneration()
{
    const uint2 screenSize = DispatchRaysDimensions().xy;
    const uint2 screenCoords = DispatchRaysIndex().xy;
    uint sampleIdx = GetSampleIndexFromScreenCoords(screenCoords, screenSize);

#if DOWNSAMPLE_FACTOR != 1
    const float2 stochasticOffsets = (CMJ_Sample2D(sampleIdx, 1024, 1024, g_GlobalConstants.m_FrameNumber + 400.0f) - 0.5f) * 2.0f;
    const uint2 sampleCoords = round( GetSampleCoordsFromScreenCoords(screenCoords) + stochasticOffsets);
    sampleIdx = GetSampleIndexFromSampleCoords(sampleCoords, screenSize / (float)DOWNSAMPLE_FACTOR);
#endif

    ShadingSurface surface = GetShadingSurfaceFromGBuffers(screenCoords, g_GBufferA, g_GBufferB, g_GBufferC, g_GBufferD);
    GIReservoir finalReservoir = GIReservoir::Unpack(g_InputReservoir[sampleIdx]);
    DeviceMemoryBarrier();

    const RayPayload shadowRay = TraceShadowRay(surface, g_GlobalConstants.m_SunDirection.xyz);
    const float3 Li = shadowRay.m_Radiance;
    const float3 wi = normalize(g_GlobalConstants.m_SunDirection.xyz);
    const float3 wo = normalize(g_GlobalConstants.m_CameraPosition.xyz - surface.m_Position);
    const float3 directLighting = ComputeRadiance(surface, Li, wi, wo);
    float3 indirectLighting = 0;

    if (finalReservoir.IsValid())
    {
        //const RayPayload validationRay = TraceValidationRay(surface, finalReservoir.m_Sample);

        //if (validationRay.m_Hit)
        //{
        //    finalReservoir = GIReservoir::Empty();
        //    g_RWOutputReservoir[sampleIdx] = GIReservoir::Pack(finalReservoir);
        //}
        
        finalReservoir.FinalizeResampling();
        indirectLighting = ComputeTargetFunction(surface, finalReservoir.m_Sample) * finalReservoir.m_WeightSum;
    }

    g_LightingOutput[screenCoords].xyz = surface.m_Emission + directLighting + indirectLighting;
    g_LightingOutput[screenCoords].a = 0;

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

    const RayPayload shadowRay = TraceShadowRay(surface, g_GlobalConstants.m_SunDirection.xyz);
    const float3 wo = normalize(-WorldRayDirection());
    const float3 wi = normalize(g_GlobalConstants.m_SunDirection.xyz);
    payload.m_Radiance = ComputeRadiance(surface, shadowRay.m_Radiance, wi, wo);
}
