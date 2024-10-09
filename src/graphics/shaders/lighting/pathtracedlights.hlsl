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
#include "utils/helpers.hlsl"
#include "utils/sampling.hlsl"
#include "utils/raytracing.hlsl"
#include "utils/encoding.hlsl"
#include "lighting/brdf.hlsl"

#define EMISSION_SCALE              8000
#define SUNLIGHT_SCALE              100000
#define SKYLIGHT_SCALE              3000
#define POINTLIGHT_SCALE            2000
#define MAX_RAY_DEPTH 3

// 0 -> Importance sample BRDF
// 1 -> Importance sample Cosine Hemisphere
// 2 -> Sample Hemisphere
#define IMPORTANCE_SAMPLING 0

ConstantBuffer<GlobalConstants> g_GlobalConstants   : register(b0);
RaytracingAccelerationStructure g_RaytracingTlas    : register(t0);
StructuredBuffer<GeometryInfo> g_GeometryInfo       : register(t1);
StructuredBuffer<Material> g_MaterialTable          : register(t2);
Texture2D<float4> g_AccumulationTexture             : register(t3);
Texture2D<float4> g_GBufferOutput0                  : register(t4);
Texture2D<float4> g_GBufferOutput1                  : register(t5);
Texture2D<float4> g_GBufferOutput2                  : register(t6);
Texture2D<float4> g_GBufferOutput3                  : register(t7);
RWTexture2D<float4> g_LightingOutput                : register(u0);
RWTexture2D<float4> g_IndirectOutput                : register(u1);

float3 EvaluateSkyLighting(float3 wi)
{
    sampler linearSampler = SamplerDescriptorHeap[g_GlobalConstants.m_SamplerIndex_Linear_Wrap];
    Texture2D<float4> hdriTexture = ResourceDescriptorHeap[g_GlobalConstants.m_HdriTextureIndex];
    const float exposure = SKYLIGHT_SCALE;

    const float2 hdriUv = SampleSphericalMap(wi);
    const float4 hdri = hdriTexture.SampleLevel(linearSampler, hdriUv, 0);
    const float sunsetFactor = saturate(asin(dot(g_GlobalConstants.m_SunDirection.xyz, float3(0, 1, 0))));
    const float sunlightFactor = 1 - saturate(asin(dot(g_GlobalConstants.m_SunDirection.xyz, float3(0, -1, 0))));

    const float4 color = lerp(float4(0.5, 0.25, 0.25, 0), 1, sunsetFactor) * sunlightFactor;

    return (exposure * hdri * color).xyz;
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

float3 TraceShadow(float3 position, float3 wo, float3 normal, float3 albedo, float roughness, float metalness, float isPortal)
{
    RayPayload payload;
    payload.m_IsShadowRay = true;

    // Flip normal if backface. This fixes a lot of light leakage
    normal = dot(wo, normal) < 0 ? -normal : normal; 

    RayDesc shadowRay;
    shadowRay.Direction = normalize(g_GlobalConstants.m_SunDirection).xyz;
    shadowRay.Origin = position + normal * 0.01;
    shadowRay.TMax = 40;
    shadowRay.TMin = 0.01;
    TraceRay(g_RaytracingTlas, RAY_FLAG_CULL_BACK_FACING_TRIANGLES | RAY_FLAG_FORCE_OPAQUE | RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH, 0xFF, 0, 0, 0, shadowRay, payload);

    const float3 wi = normalize(shadowRay.Direction);
    const float3 Li = payload.m_Radiance;
    const float3 f = BRDF_UE4(wi, wo, normal, albedo, roughness, metalness);
    const float cosTheta = saturate(dot(wi, normal));
    const float3 Lo = Li * f * cosTheta;


    const float3 portalColor = float3(0, 0.3, 1) * POINTLIGHT_SCALE;
    const float3 portalPosition1 = float3(0, 0.85, 0.18792);
    const float portalDistance1 = abs(length(portalPosition1 - position));
    const float3 portalWi1 = normalize(portalPosition1 - position);
    const float portalNoL1 = dot(normal, portalWi1);
    const float3 portalLi1 = (portalNoL1 * BRDF_UE4(portalWi1, wo, normal, albedo, roughness, metalness) * portalColor) / pow(portalDistance1, 2.5);

    const float3 portalPosition2 = float3(0, 0.85 - 20, 0.18792);
    const float portalDistance2 = abs(length(portalPosition2 - position));
    const float3 portalWi2 = normalize(portalPosition2 - position);
    const float portalNoL2 = dot(normal, portalWi2);
    const float3 portalLi2 = (portalNoL2 * BRDF_UE4(portalWi2, wo, normal, albedo, roughness, metalness) * portalColor) / pow(portalDistance2, 2.5);

    return Lo + portalLi1 + portalLi2;
}

// 0 -> Importance sample BRDF
// 1 -> Importance sample Cosine Hemisphere
// 2 -> Importance sample Hemisphere
#define IMPORTANCE_SAMPLING 0

float3 TraceRecursively(float3 position, float3 wo, float3 normal, float3 albedo, float roughness, float metalness, uint depth, float isPortal)
{
    if (depth <= 0)
    {
        return EvaluateSkyLighting(WorldRayDirection());
    }

    const uint3 launchIndex = DispatchRaysIndex();
    const uint3 launchDim = DispatchRaysDimensions();
    const uint sampleIdx = launchIndex.y * launchDim.x + launchIndex.x;
    const float2 rand2D = CMJ_Sample2D(sampleIdx, 1024, 1024, g_GlobalConstants.m_FrameNumber);

    // Flip normal if backface. This fixes a lot of light leakage
    normal = dot(wo, normal) < 0 ? -normal : normal; 

#if IMPORTANCE_SAMPLING == 0
    const float diffuseWeight = lerp(lerp(0.5, 1.0, roughness), 0.0, metalness);
    const float specularWeight = 1.0 - diffuseWeight;
    float3 wi = 0;
    const bool importanceSampleBrdf = Random(rand2D.x) <= specularWeight;

    if (importanceSampleBrdf)
        wi = normalize(ImportanceSampleGGX(rand2D, wo, normal, roughness));
    else
        wi = TangentToWorld(SampleDirectionCosineHemisphere(rand2D), normal);

    if (isPortal)
        wi = -wo;

    const float3 H = normalize(wi + wo);
    const float nDotH = saturate(dot(normal, H));
    const float nDotV = saturate(dot(normal, wo));
    const float vDotH = saturate(dot(wo, H));
    const float cosTheta = saturate(dot(wi, normal));
    float pdf;

    if (importanceSampleBrdf)
        pdf = UE4JointPdf(specularWeight, nDotH, cosTheta, vDotH, roughness);
    else
        pdf = SampleDirectionHemisphere_Pdf();

    if (isPortal)
        pdf = 1;

    float3 f = 0;
    if (cosTheta > 0 && pdf > 0)
        f = BRDF_UE4(wi, wo, normal, albedo, roughness, metalness) / pdf;


#elif IMPORTANCE_SAMPLING == 1
    float3 wi = TangentToWorld(SampleDirectionCosineHemisphere(rand2D), normal);
    const float cosTheta = saturate(dot(wi, normal));
    const float pdf = cosTheta / Pi;

    float3 f = 0;
    if (cosTheta > 0 && pdf > 0)
        f = BRDF_UE4(wi, wo, normal, albedo, roughness, metalness) / pdf;
#else
    float3 wi = TangentToWorld(SampleDirectionHemisphere(rand2D), normal);
    const float cosTheta = saturate(dot(wi, normal));
    const float pdf = 1 / Pi2;

    float3 f = 0;
    if (cosTheta > 0 && pdf > 0)
        f = BRDF_UE4(wi, wo, normal, albedo, roughness, metalness) / pdf;
#endif

    RayPayload payload;
    payload.m_IsShadowRay = false;
    payload.m_Depth = depth;

    RayDesc indirectRay;
    indirectRay.Direction = wi;
    indirectRay.Origin = position;
    indirectRay.TMax = 40;
    indirectRay.TMin = isPortal ? 0.0001 : 0.1;
    TraceRay(g_RaytracingTlas, RAY_FLAG_FORCE_OPAQUE | RAY_FLAG_CULL_BACK_FACING_TRIANGLES, 0xFF, 0, 0, 0, indirectRay, payload);
    float3 Li = min(20000, payload.m_Radiance);
    const float3 Lo = Li * f * cosTheta;

    if (isPortal)
        return payload.m_Radiance;

    return Lo;
}

float3 PathTrace(in MeshVertex hitSurface, in Material material, inout RayPayload payload)
{

    sampler linearSampler = SamplerDescriptorHeap[g_GlobalConstants.m_SamplerIndex_Linear_Wrap];
    const uint mipLevelToSample = 0;

    float4 albedo = material.m_BaseColor;
    float4 emission = material.m_EmissiveColor * EMISSION_SCALE;
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

    roughness = 1;

    if (material.m_MetalnessTextureIndex != 0)
    {
        Texture2D<float4> metalnessTex = ResourceDescriptorHeap[material.m_MetalnessTextureIndex];
        metalness = metalnessTex.SampleLevel(linearSampler, hitSurface.m_TexCoord, mipLevelToSample).b;
    }

    if (material.m_EmissiveTextureIndex != 0)
    {
        Texture2D<float4> emissiveTex = ResourceDescriptorHeap[material.m_EmissiveTextureIndex];
        emission *= pow(emissiveTex.SampleLevel(linearSampler, hitSurface.m_TexCoord, mipLevelToSample), 4.0);
    }

    // Flip normal if backface. This fixes a lot of light leakage
    normal = dot(WorldRayDirection(), hitSurface.m_Normal) > 0 ? -normal : normal;

    if (hitSurface.m_Position.y <= -20.1)
    {
        roughness = 0.1;
        metalness = 1;
        albedo *= 1;
    }

    if ((hitSurface.m_Position.y <= 21.435 && hitSurface.m_Position.y >= 21.0))
    {
        roughness = 0.1;
        metalness = 0;
        albedo = float4(0.1, 0.2, 0.6, 0) / 6;
    }

    const float3 wo = normalize(-WorldRayDirection());
    const float3 direct = TraceShadow(hitSurface.m_Position, wo, normal, albedo.xyz, roughness, metalness, 0.0);
    const float3 indirect = TraceRecursively(hitSurface.m_Position, wo, normal, albedo.xyz, roughness, metalness, payload.m_Depth - 1, 0.0);
    return emission.xyz + direct + indirect;
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
    const float4 gbuffer3 = g_GBufferOutput3.Load(launchIndex);

    const float3 color = gbuffer0.xyz;
    const float3 emission = gbuffer3.xyz * EMISSION_SCALE;
    float3 position = gbuffer1.xyz;
    const float3 viewDir = normalize(g_GlobalConstants.m_CameraPosition.xyz - position);
    const float3 normal = normalize(DecodeNormals(gbuffer2.xy));
    const float2 velocity = gbuffer2.zw;
    const float metalness = gbuffer0.w;
    const float roughness = gbuffer1.w;
    const float2 uv = (float2)launchIndex.xy / launchDim.xy + rcp((float2)launchDim.xy) / 2.0;
    const float2 uvPrev = uv - velocity;
    float4 accumulation = 0;

    if (uvPrev.x >= 0 && uvPrev.x < 1 && uvPrev.y >= 0 && uvPrev.y < 1)
         accumulation = g_AccumulationTexture.SampleLevel(linearSampler, uvPrev, 0);

    float3 direct = 0;
    float3 indirect = 0;

    float isPortal = gbuffer3.w;

    if (!isPortal)
        direct = TraceShadow(position, viewDir, normal, color, roughness, metalness, isPortal);

    if (isPortal)
    {
        if (viewDir.z >= 0)
            position.y += 20.0;
        else
            position.y -= 20.0;

        if (isPortal < 0.6)
        {
            position.x -= 1.0;
            position.y += 19.2;
            position.z -= 39.0;
        }
    }


    indirect = TraceRecursively(position, viewDir, normal, color, roughness, metalness, MAX_RAY_DEPTH, isPortal);

    float a = 0.025;// max(0.01, 1 - smoothstep(0, 10, g_GlobalConstants.m_FrameNumber - g_GlobalConstants.m_FrameSinceLastMovement));
    const float3 accumulatedIndirect = (a * indirect) + (1 - a) * accumulation.xyz;
    g_LightingOutput[launchIndex.xy].xyz = emission + direct + accumulatedIndirect;
    g_IndirectOutput[launchIndex.xy].xyz = accumulatedIndirect;
}

[shader("miss")]
void Miss(inout RayPayload payload)
{
    payload.m_Hit = false;
    payload.m_Radiance = 0;

    if (payload.m_IsShadowRay)
    {
        // Sample sun color
        payload.m_Radiance = g_GlobalConstants.m_SunColor.xyz * lerp(0.0f, SUNLIGHT_SCALE, saturate(dot(g_GlobalConstants.m_SunDirection.xyz, float3(0, 1, 0))));
    }
    else
    {
        // Sample sky color
        payload.m_Radiance = EvaluateSkyLighting(WorldRayDirection());
    }
}

[shader("closesthit")]
void ClosestHit(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attribs)
{
    payload.m_Hit = true;
    payload.m_Radiance = 0;

    const GeometryInfo geoInfo = g_GeometryInfo[InstanceIndex()];
    const MeshVertex vertex = GetHitSurface(attribs, geoInfo);
    const Material material = g_MaterialTable[geoInfo.m_MaterialIndex];


    // [PORTAL]
    if ((vertex.m_Position.z >= 0.187 && vertex.m_Position.z <= 0.188 && vertex.m_Normal.z >= 0.99) ||
        (vertex.m_Position.x >= -0.3175 && vertex.m_Position.x <= -0.3165 && vertex.m_Position.z > 9.5 && vertex.m_Position.z < 10.2 && abs(vertex.m_Normal.x) >= 0.99)
        )
    {
        payload.m_Radiance = float3(0.5, 0.75, 1.0) * 50000;// TraceRecursively(hitSurface.m_Position, normalize(-WorldRayDirection()), normalize(-WorldRayDirection()), 1, 0, 0, payload.m_Depth - 1, true);
        return;
    }

    if (payload.m_IsShadowRay)
        return;


    payload.m_Radiance = PathTrace(vertex, material, payload);
}

