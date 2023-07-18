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
#include "utils/encoding.hlsl"
#include "lighting/brdf.hlsl"

ConstantBuffer<GlobalConstants> g_GlobalConstants   : register(b0);
RaytracingAccelerationStructure g_RaytracingTlas    : register(t0);
StructuredBuffer<GeometryInfo> g_GeometryInfo       : register(t1);
StructuredBuffer<Material> g_MaterialTable          : register(t2);
Texture2D<float4> g_AccumulationTexture             : register(t3);
Texture2D<float4> g_GBufferOutput0                  : register(t4);
Texture2D<float4> g_GBufferOutput1                  : register(t5);
Texture2D<float4> g_GBufferOutput2                  : register(t6);
RWTexture2D<float4> g_LightingOutput                : register(u0);
RWTexture2D<float4> g_IndirectOutput                : register(u1);

float3 ComputeSkyColor()
{
    float3 daySkyColor = float3(0.6, 0.7, 1.0) * 0.4;
    float3 nightSkyColor = float3(0.1, 0.25, 0.4) * 0.1;
    float3 skyColor = lerp(nightSkyColor, daySkyColor, saturate(dot(g_GlobalConstants.m_SunDirection.xyz, float3(0, 1, 0))));
    return skyColor * 0.1;
}

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

float3 ComputeDirectIrradiance(float3 position, float3 wo, float3 normal, float3 albedo, float roughness, float metalness)
{
    RayPayload payload;
    payload.m_IsShadowRay = true;

    RayDesc shadowRay;
    shadowRay.Direction = normalize(g_GlobalConstants.m_SunDirection).xyz;
    shadowRay.Origin = position;
    shadowRay.TMax = 64;
    shadowRay.TMin = 0.05;
    TraceRay(g_RaytracingTlas, RAY_FLAG_FORCE_OPAQUE | RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH, 0xFF, 0, 0, 0, shadowRay, payload);

    const float3 wi = normalize(shadowRay.Direction);
    const float3 Li = payload.m_Radiance;
    const float3 f = UE4_Brdf(wi, wo, normal, albedo, roughness, metalness);
    const float cosTheta = saturate(dot(wi, normal));
    const float3 Lo = Li * f * cosTheta;
    return Lo;
}

float3 ComputeIndirectIrradiance(float3 position, float3 wo, float3 normal, float3 albedo, float roughness, float metalness, uint depth)
{
    const float3 ambient = ComputeSkyColor() * 0.1;

    if (depth <= 0)
        return ambient;

    const uint3 launchIndex = DispatchRaysIndex();
    const uint3 launchDim = DispatchRaysDimensions();
    const uint sampleIdx = launchIndex.y * launchDim.x + launchIndex.x;

    const float2 rand2D = CMJ_Sample2D(sampleIdx, 1024, 1024, g_GlobalConstants.m_FrameNumber);
    float3 direction = normalize(ImportanceSampleGGX(rand2D, roughness, normal));
    float pdf = GGX_PDF(dot(normal, direction), roughness);

    const float3 sphere = normalize(SampleDirectionSphere(rand2D));
    const float3 mirror = reflect(-wo, normal);
    direction = normalize(mirror) + sphere * roughness;

    RayPayload payload;
    payload.m_IsShadowRay = false;
    payload.m_Depth = depth;

    RayDesc indirectRay;
    indirectRay.Direction = direction;
    indirectRay.Origin = position + normal;
    indirectRay.TMax = 12;
    indirectRay.TMin = 0.01;
    TraceRay(g_RaytracingTlas, RAY_FLAG_FORCE_OPAQUE, 0xFF, 0, 0, 0, indirectRay, payload);

    const float3 wi = direction;
    const float3 Li = payload.m_Radiance;
    const float cosTheta = dot(wi, normal);
    const float indirectBoost = (g_GlobalConstants.m_RaytracedAOIntensity * 5);
    const float3 Lo = Li * albedo * indirectBoost; 
    return Lo + ambient;
}

float3 PathTrace(in MeshVertex hitSurface, in Material material, in RayPayload payload)
{
    sampler linearSampler = SamplerDescriptorHeap[g_GlobalConstants.m_SamplerIndex_Linear_Wrap];
    const uint mipLevelToSample = 99;

    float4 albedo = material.m_BaseColor;
    float3 normal = hitSurface.m_Normal;
    float roughness = 1;
    float metalness = 0;

    if (material.m_AlbedoTextureIndex != 0)
    {
        Texture2D<float4> albedoTex = ResourceDescriptorHeap[material.m_AlbedoTextureIndex];
        albedo *= albedoTex.SampleLevel(linearSampler, hitSurface.m_TexCoord, mipLevelToSample);
    }

    if (material.m_NormalTextureIndex != 0)
    {
        Texture2D<float4> normalTex = ResourceDescriptorHeap[material.m_NormalTextureIndex];
        normal = normalTex.SampleLevel(linearSampler, hitSurface.m_TexCoord, mipLevelToSample).xyz;
        normal = normal * 2.0 - 1.0;
        float3 bitangent = cross(hitSurface.m_Tangent, hitSurface.m_Normal);
        float3x3 TBN = float3x3(hitSurface.m_Tangent, bitangent, hitSurface.m_Normal.xyz);
        normal = normalize(mul(normal, TBN));
    }

    if (material.m_RoughnessTextureIndex != 0)
    {
        Texture2D<float4> roughnessTex = ResourceDescriptorHeap[material.m_RoughnessTextureIndex];
        roughness = roughnessTex.SampleLevel(linearSampler, hitSurface.m_TexCoord, mipLevelToSample).g;
    }

    if (material.m_MetalnessTextureIndex != 0)
    {
        Texture2D<float4> metalnessTex = ResourceDescriptorHeap[material.m_MetalnessTextureIndex];
        metalness = metalnessTex.SampleLevel(linearSampler, hitSurface.m_TexCoord, mipLevelToSample).b;
    }

    const float3 wo = normalize(-WorldRayDirection());
    const float3 sunIrradiance = ComputeDirectIrradiance(hitSurface.m_Position, wo, normal, albedo.xyz, roughness, metalness);
    const float3 bounceIrradiance = ComputeIndirectIrradiance(hitSurface.m_Position, wo, normal, albedo.xyz, roughness, metalness, payload.m_Depth - 1);
    return sunIrradiance + bounceIrradiance;
}

[shader("raygeneration")]
void RayGeneration()
{
    sampler linearSampler = SamplerDescriptorHeap[g_GlobalConstants.m_SamplerIndex_Linear_Clamp];

    const uint3 launchIndex = DispatchRaysIndex();
    const uint3 launchDim = DispatchRaysDimensions();
    const uint sampleIdx = launchIndex.y * launchDim.x + launchIndex.x;

    const float4 gbuffer0 = g_GBufferOutput0.Load(launchIndex);
    const float4 gbuffer1 = g_GBufferOutput1.Load(launchIndex);
    const float4 gbuffer2 = g_GBufferOutput2.Load(launchIndex);

    const float3 color = gbuffer0.xyz;
    const float3 position = gbuffer1.xyz;
    const float3 viewDir = normalize(g_GlobalConstants.m_EyePosition.xyz - position);
    const float3 normal = normalize(DecodeNormals(gbuffer2.xy));
    const float2 velocity = gbuffer2.zw;
    const float metalness = gbuffer0.w;
    const float roughness = gbuffer1.w;
    const float2 uv = (float2)launchIndex.xy / launchDim.xy + rcp((float2)launchDim.xy) / 2.0;
    const float2 uvPrev = uv - velocity;
    const float4 accumulation = g_AccumulationTexture.SampleLevel(linearSampler, uvPrev, 0);

    const float3 directIrradiance = ComputeDirectIrradiance(position, viewDir, normal, color, roughness, metalness);
    const float3 indirectIrradiance = ComputeIndirectIrradiance(position, viewDir, normal, color, roughness, metalness, 2);

    float a = 0.1;
    const float3 accumulatedDiffuseIndirect = (a * indirectIrradiance) + (1 - a) * accumulation.xyz;
    g_LightingOutput[launchIndex.xy].xyz = directIrradiance + accumulatedDiffuseIndirect;
    g_IndirectOutput[launchIndex.xy].xyz = accumulatedDiffuseIndirect;
}

[shader("miss")]
void Miss(inout RayPayload payload)
{
    payload.m_Hit = false;
    payload.m_Radiance = 0;

    if (payload.m_IsShadowRay)
    {
        // Sample sun color
        payload.m_Radiance = g_GlobalConstants.m_SunColor.xyz * 10.0;
    }
    else
    {
        // Sample sky color
        payload.m_Radiance = ComputeSkyColor() * 8.0;
    }
}

[shader("closesthit")]
void ClosestHit(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attribs)
{
    payload.m_Hit = true;
    payload.m_Radiance = 0;

    if (payload.m_IsShadowRay)
        return;

    const GeometryInfo geoInfo = g_GeometryInfo[InstanceIndex()];
    const MeshVertex vertex = GetHitSurface(attribs, geoInfo);
    const Material material = g_MaterialTable[geoInfo.m_MaterialIndex];

    payload.m_Radiance = PathTrace(vertex, material, payload);
}
