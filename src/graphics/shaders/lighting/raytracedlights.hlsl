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

#include "common/globalconstants.h"
#include "common/raytracingconstants.h"
#include "common/material.h"
#include "utils/sampling.hlsl"
#include "utils/raytracing.hlsl"

ConstantBuffer<GlobalConstants> g_GlobalConstants   : register(b0);
RaytracingAccelerationStructure g_RaytracingTlas    : register(t0);
StructuredBuffer<GeometryInfo> g_GeometryInfo       : register(t1);
StructuredBuffer<Material> g_MaterialTable          : register(t2);
Texture2D<float4> g_GBufferOutput0                  : register(t3);
Texture2D<float4> g_GBufferOutput1                  : register(t4);
Texture2D<float4> g_GBufferOutput2                  : register(t5);
Texture2D<float4> g_GBufferOutput3                  : register(t6);
RWTexture2D<float4> g_LightingOutput                : register(u0);

MeshVertex GetHitSurface(in BuiltInTriangleIntersectionAttributes attribs, in GeometryInfo geoInfo)
{
    float3 barycentrics;
    barycentrics.x = 1 - attribs.barycentrics.x - attribs.barycentrics.y;
    barycentrics.y = attribs.barycentrics.x;
    barycentrics.z = attribs.barycentrics.y;

    StructuredBuffer<MeshVertex> vtxBuffer = ResourceDescriptorHeap[geoInfo.m_VBDescriptorIndex];
    Buffer<uint> idxBuffer = ResourceDescriptorHeap[geoInfo.m_IBDescriptorIndex];

    const uint primIdx = PrimitiveIndex();
    const uint idx0 = idxBuffer[primIdx * 3 + 0];
    const uint idx1 = idxBuffer[primIdx * 3 + 1];
    const uint idx2 = idxBuffer[primIdx * 3 + 2];

    const MeshVertex v0 = vtxBuffer[idx0];
    const MeshVertex v1 = vtxBuffer[idx1];
    const MeshVertex v2 = vtxBuffer[idx2];

    return BarycentricLerp(v0, v1, v2, barycentrics);
}

float3 ComputeDirectLighting(float3 position, float3 normal)
{
    RayPayload payload;
    payload.m_IsShadowRay = true;

    RayDesc shadowRay;
    shadowRay.Direction = normalize(g_GlobalConstants.m_SunDirection).xyz;
    shadowRay.Origin = position;
    shadowRay.TMax = 64;
    shadowRay.TMin = 0.01;
    TraceRay(g_RaytracingTlas, 0, 0xFF, 0, 0, 0, shadowRay, payload);

    return saturate(dot(normal, shadowRay.Direction)) * payload.m_Radiance;
}

float3 ComputeIndirectLighting(float3 position, float3 normal, float roughness, uint depth)
{
    const uint3 launchIndex = DispatchRaysIndex();
    const uint3 launchDim = DispatchRaysDimensions();
    const uint sampleIdx = launchIndex.y * launchDim.x + launchIndex.x;

    const float3 ranSphere = SampleDirectionSphere(CMJ_Sample2D(sampleIdx, launchDim.x, launchDim.x, g_GlobalConstants.m_FrameNumber));
    const float3 ranDirection = normalize(normal + roughness * ranSphere);

    RayPayload payload;
    payload.m_IsShadowRay = false;
    payload.m_Depth = depth;

    RayDesc indirectRay;
    indirectRay.Direction = ranDirection;
    indirectRay.Origin = position;
    indirectRay.TMax = 64;
    indirectRay.TMin = 0.01;

    TraceRay(g_RaytracingTlas, 0, 0xFF, 0, 0, 0, indirectRay, payload);
    return saturate(dot(normal, indirectRay.Direction)) * payload.m_Radiance;
}

float3 ComputeSkyColor()
{
    float3 sunColor = g_GlobalConstants.m_SunColor.xyz;
    float3 nightSkyColor = float3(0.1, 0.25, 0.4);
    float3 daySkyColor = float3(0.6, 0.7, 1.0);
    float3 skyColor = lerp(nightSkyColor, daySkyColor, dot(g_GlobalConstants.m_SunDirection.xyz, float3(0, 1, 0)));
    return skyColor;
}

float3 PathTrace(in MeshVertex hitSurface, in Material material, in RayPayload payload)
{
    sampler linearSampler = SamplerDescriptorHeap[g_GlobalConstants.m_SamplerIndex_Linear_Wrap];

    float4 albedo = material.m_BaseColor;
    float3 normal = hitSurface.m_Normal;
    float roughness = 0.5;
    float metalness = 0;

    if (material.m_AlbedoTextureIndex != 0)
    {
        Texture2D<float4> albedoTex = ResourceDescriptorHeap[material.m_AlbedoTextureIndex];
        payload.m_Radiance *= albedoTex.SampleLevel(linearSampler, hitSurface.m_TexCoord, 0).xyz;
    }

    if (material.m_NormalTextureIndex != 0)
    {
        Texture2D<float4> normalTex = ResourceDescriptorHeap[material.m_NormalTextureIndex];
        normal = normalTex.SampleLevel(linearSampler, hitSurface.m_TexCoord, 0).xyz;
        normal = normal * 2.0 - 1.0;
        float3x3 TBN = float3x3(hitSurface.m_Tangent, hitSurface.m_Bitangent, hitSurface.m_Normal.xyz);
        normal = normalize(mul(normal, TBN));
    }

    //if (material.m_RoughnessTextureIndex != 0)
    //{
    //    Texture2D<float4> roughnessTex = ResourceDescriptorHeap[material.m_RoughnessTextureIndex];
    //    roughness = roughnessTex.SampleLevel(linearSampler, hitSurface.m_TexCoord, 0).g;
    //}

    //if (material.m_MetalnessTextureIndex != 0)
    //{
    //    Texture2D<float4> metalnessTex = ResourceDescriptorHeap[material.m_MetalnessTextureIndex];
    //    metalness = metalnessTex.SampleLevel(linearSampler, hitSurface.m_TexCoord, 0).b;
    //}

    float3 radiance = payload.m_Radiance;
    radiance += ComputeDirectLighting(hitSurface.m_Position, normal) * albedo.xyz;
    radiance += ComputeIndirectLighting(hitSurface.m_Position, normal, roughness, payload.m_Depth) * albedo.xyz;

    return radiance;
}

[shader("raygeneration")]
void RayGeneration()
{
    sampler linearSampler = SamplerDescriptorHeap[g_GlobalConstants.m_SamplerIndex_Linear_Clamp];

    const uint3 launchIndex = DispatchRaysIndex();
    const uint3 launchDim = DispatchRaysDimensions();
    const float2 uv = (float2)launchIndex.xy / launchDim.xy + rcp((float2)launchDim.xy) / 2.0;
    const uint sampleIdx = launchIndex.y * launchDim.x + launchIndex.x;

    const float4 gbuffer1 = g_GBufferOutput1.Load(launchIndex);
    const float4 gbuffer2 = g_GBufferOutput2.Load(launchIndex);

    const float3 position = gbuffer1.xyz;
    const float3 normal = gbuffer2.xyz;
    const float roughness = gbuffer1.w;
    const float metalness = gbuffer2.w;

    const float3 ambient = ComputeSkyColor() * 0.1;

    float3 finalRadiance = 0;
    finalRadiance += ambient;
    finalRadiance += ComputeDirectLighting(position, normal);
    finalRadiance += ComputeIndirectLighting(position, normal, roughness, 1) * g_GlobalConstants.m_RaytracedAOIntensity * 2;

    g_LightingOutput[launchIndex.xy].xyz = finalRadiance.xyz;
}

[shader("miss")]
void Miss(inout RayPayload payload)
{
    payload.m_Hit = false;

    if (payload.m_IsShadowRay)
    {
        // Sample sun color
        payload.m_Radiance = g_GlobalConstants.m_SunColor.xyz * 2;
        return;
    }

    // Sample sky color
    payload.m_Radiance = ComputeSkyColor() * 4;
}

[shader("closesthit")]
void ClosestHit(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attribs)
{
    payload.m_Hit = true;
    payload.m_HitT = RayTCurrent();

    if (payload.m_IsShadowRay || payload.m_Depth <= 0)
    {
        payload.m_Radiance = 0;
        return;
    }

    const GeometryInfo geoInfo = g_GeometryInfo[InstanceIndex()];
    const MeshVertex vertex = GetHitSurface(attribs, geoInfo);
    const Material material = g_MaterialTable[geoInfo.m_MaterialIndex];

    payload.m_Depth -= 1;
    payload.m_Radiance = PathTrace(vertex, material, payload);
}
