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

#include "utils/helpers.hlsl"
#include "utils/sampling.hlsl"
#include "utils/raytracing.hlsl"

Texture2D<float4> g_AccumulationTexture             : register(t3);
Texture2D<float4> g_GBufferA                        : register(t4);
Texture2D<float4> g_GBufferB                        : register(t5);
Texture2D<float4> g_GBufferC                        : register(t6);
Texture2D<float4> g_GBufferD                        : register(t7);
RWTexture2D<float4> g_LightingOutput                : register(u0);
RWTexture2D<float4> g_IndirectOutput                : register(u1);

[shader("raygeneration")]
void RayGeneration()
{
    const uint2 sampleCoords = DispatchRaysIndex().xy;
    const uint2 bufferSize = DispatchRaysDimensions().xy;
    const uint sampleIdx = sampleCoords.y * bufferSize.x + sampleCoords.x;
    const ShadingSurface surface = GetShadingSurfaceFromGBuffers(sampleCoords, g_GBufferA, g_GBufferB, g_GBufferC, g_GBufferD);

    const float3 viewDir = normalize(g_GlobalConstants.m_CameraPosition.xyz - surface.m_Position);
    const float2 uv = (float2) sampleCoords.xy / bufferSize.xy + rcp((float2) bufferSize.xy) / 2.0;
    const float2 uvPrev = uv - surface.m_Velocity;
    float4 accumulation = 0;

    sampler linearSampler = SamplerDescriptorHeap[g_GlobalConstants.m_SamplerIndex_Linear_Clamp];
    if (all(uv >= 0.0f) && all(uv <= 1.0f))
         accumulation = g_AccumulationTexture.SampleLevel(linearSampler, uvPrev, 0);

    const RayPayload shadowRay = TraceShadowRay(surface, g_GlobalConstants.m_SunDirection.xyz);
    const float3 direct = ComputeRadiance(surface, shadowRay.m_Radiance, g_GlobalConstants.m_SunDirection.xyz, viewDir);

    float3 wi;
    float pdf;

#if USE_IMPORTANCE_SAMPLING
    SampleDirectionBrdf(surface, g_GlobalConstants.m_FrameNumber, viewDir, wi, pdf);
#else
    SampleDirectionUniform(surface, g_GlobalConstants.m_FrameNumber, wi, pdf);
#endif

    const RayPayload indirectRay = TraceShadingRay(surface, wi, MAX_DEPTH);
    const float3 indirect = ComputeRadiance(surface, indirectRay.m_Radiance, wi, viewDir) / pdf;

    float a = max(0.01, 1 - smoothstep(0, 10, g_GlobalConstants.m_FrameNumber - g_GlobalConstants.m_FrameSinceLastMovement));
    const float3 accumulatedIndirect = (a * indirect) + (1 - a) * accumulation.xyz;
    g_LightingOutput[sampleCoords].xyz = surface.m_Emission + direct + accumulatedIndirect;
    g_IndirectOutput[sampleCoords].xyz = accumulatedIndirect;
}

[shader("closesthit")]
void ClosestHit(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attribs)
{
    const GeometryInfo geoInfo = g_GeometryInfo[InstanceIndex()];
    const MeshVertex vertex = GetHitSurface(attribs, geoInfo);
    const Material material = g_MaterialTable[geoInfo.m_MaterialIndex];
    const ShadingSurface surface = GetShadingSurfaceFromHit(vertex, material, g_GlobalConstants.m_SamplerIndex_Linear_Wrap, INDIRECT_MIP_LEVEL);
    const float3 viewDir = normalize(g_GlobalConstants.m_CameraPosition.xyz - surface.m_Position);

    payload.m_Hit = true;
    payload.m_HitPosition = surface.m_Position;
    payload.m_HitNormal = surface.m_Normal;
    payload.m_Depth = max(0, (int) payload.m_Depth - 1);
    payload.m_Radiance = 0;

    if (payload.m_Depth <= 0)
        return;

    if (payload.m_IsShadowRay)
        return;

    float3 direct;
    float3 indirect;

    {   // Direct lighting
        const RayPayload shadowRay = TraceShadowRay(surface, g_GlobalConstants.m_SunDirection.xyz);
        direct = ComputeRadiance(surface, shadowRay.m_Radiance, g_GlobalConstants.m_SunDirection.xyz, -WorldRayDirection());
    }

    {   // Indirect lighting
        float3 wi;
        float pdf;

#if USE_IMPORTANCE_SAMPLING
        SampleDirectionBrdf(surface, g_GlobalConstants.m_FrameNumber, viewDir, wi, pdf);
#else
        SampleDirectionUniform(surface, g_GlobalConstants.m_FrameNumber, wi, pdf);
#endif
        
        const RayPayload indirectRay = TraceShadingRay(surface, wi, payload.m_Depth);
        indirect = ComputeRadiance(surface, indirectRay.m_Radiance, wi, -WorldRayDirection()) / pdf;
    }

    payload.m_Radiance = surface.m_Emission + direct + indirect;
}
