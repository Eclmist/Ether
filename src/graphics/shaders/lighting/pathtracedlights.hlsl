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
#include "utils/shading.hlsl"
#include "lighting/brdf.hlsl"

#define SUNLIGHT_SCALE 1
#define SKYLIGHT_SCALE 2000

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
    const float4 hdri = hdriTexture.SampleLevel(linearSampler, hdriUv, 4);
    const float sunsetFactor = saturate(asin(dot(g_GlobalConstants.m_SunDirection.xyz, float3(0, 1, 0))));
    const float sunlightFactor = 1 - saturate(asin(dot(g_GlobalConstants.m_SunDirection.xyz, float3(0, -1, 0))));

    const float4 color = lerp(float4(0.5, 0.25, 0.25, 0), 1, sunsetFactor) * sunlightFactor;

    return (exposure * hdri * color).xyz;
}

float3 ComputeRadiance(ShadingSurface surface, float3 Li, float3 wi, float3 wo)
{
    const float3 f = BRDF_UE4(wi, wo, surface.m_Normal, surface.m_Albedo, surface.m_Roughness, surface.m_Metalness);
    const float cosTheta = saturate(dot(wi, surface.m_Normal));
    return f * Li * cosTheta;
}

RayPayload TraceShadowRay(ShadingSurface surface)
{
    RayPayload payload;
    payload.m_IsShadowRay = true;
    payload.m_Depth = 1;

    RayDesc ray;
    ray.Direction = normalize(g_GlobalConstants.m_SunDirection).xyz;
    ray.Origin = surface.m_Position + surface.m_Normal * 0.01;
    ray.TMax = RAY_TMAX;
    ray.TMin = RAY_TMIN;
    TraceRay(g_RaytracingTlas, RAY_FLAG_FORCE_OPAQUE | RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH, 0xFF, 0, 0, 0, ray, payload);

    return payload;
}

float3 TraceRecursively(float3 position, float3 wo, float3 normal, float3 albedo, float roughness, float metalness, uint depth)
{
    if (depth <= 0)
        return 0;

    const uint3 launchIndex = DispatchRaysIndex();
    const uint3 launchDim = DispatchRaysDimensions();
    const uint sampleIdx = launchIndex.y * launchDim.x + launchIndex.x;
    const float2 rand2D = CMJ_Sample2D(sampleIdx, 1024, 1024, g_GlobalConstants.m_FrameNumber);

#if IMPORTANCE_SAMPLING == 0
    const float diffuseWeight = lerp(lerp(0.5, 1.0, roughness), 0.0, metalness);
    const float specularWeight = 1.0 - diffuseWeight;
    float3 wi = 0;
    const bool importanceSampleBrdf = Random(rand2D.x) <= specularWeight;

    if (importanceSampleBrdf)
        wi = normalize(ImportanceSampleGGX(rand2D, wo, normal, roughness));
    else
        wi = TangentToWorld(SampleDirectionCosineHemisphere(rand2D), normal);

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
    indirectRay.TMax = RAY_TMAX;
    indirectRay.TMin = RAY_TMIN;
    TraceRay(g_RaytracingTlas, RAY_FLAG_FORCE_OPAQUE, 0xFF, 0, 0, 0, indirectRay, payload);
    const float3 Li = min(10000.0f, payload.m_Radiance);

    return Li * f * cosTheta;
}

[shader("raygeneration")]
void RayGeneration()
{
    const uint2 sampleCoords = DispatchRaysIndex().xy;
    const uint2 bufferSize = DispatchRaysDimensions().xy;
    const ShadingSurface surface = GetShadingSurfaceFromGBuffers(sampleCoords, g_GBufferOutput0, g_GBufferOutput1, g_GBufferOutput2, g_GBufferOutput3);

    const float3 viewDir = normalize(g_GlobalConstants.m_CameraPosition.xyz - surface.m_Position);
    const float2 uv = (float2) sampleCoords.xy / bufferSize.xy + rcp((float2) bufferSize.xy) / 2.0;
    const float2 uvPrev = uv - surface.m_Velocity;
    const float3 wo = g_GlobalConstants.m_SunDirection.xyz;
    float4 accumulation = 0;

    sampler linearSampler = SamplerDescriptorHeap[g_GlobalConstants.m_SamplerIndex_Linear_Clamp];
    if (uvPrev.x >= 0 && uvPrev.x < 1 && uvPrev.y >= 0 && uvPrev.y < 1)
         accumulation = g_AccumulationTexture.SampleLevel(linearSampler, uvPrev, 0);

    const RayPayload shadowRay = TraceShadowRay(surface);
    const float3 direct = ComputeRadiance(surface, shadowRay.m_Radiance, viewDir, wo);
    const float3 indirect = TraceRecursively(surface.m_Position, viewDir, surface.m_Normal, surface.m_Albedo, surface.m_Roughness, surface.m_Metalness, MAX_DEPTH);

    float a = max(0.01, 1 - smoothstep(0, 10, g_GlobalConstants.m_FrameNumber - g_GlobalConstants.m_FrameSinceLastMovement));
    const float3 accumulatedIndirect = (a * indirect) + (1 - a) * accumulation.xyz;
    g_LightingOutput[sampleCoords].xyz = surface.m_Emission + direct + accumulatedIndirect;
    g_IndirectOutput[sampleCoords].xyz = accumulatedIndirect;
}

[shader("miss")]
void Miss(inout RayPayload payload)
{
    payload.m_Hit = false;
    payload.m_Radiance = 0;

    if (payload.m_IsShadowRay)
    {
        // Sample sun color
        const float lerpFactor = saturate(dot(g_GlobalConstants.m_SunDirection.xyz, float3(0, 1, 0)));
        payload.m_Radiance = lerp(0.0f, g_GlobalConstants.m_SunColor.xyz, lerpFactor);
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
    const GeometryInfo geoInfo = g_GeometryInfo[InstanceIndex()];
    const MeshVertex vertex = GetHitSurface(attribs, geoInfo);
    const Material material = g_MaterialTable[geoInfo.m_MaterialIndex];

    ShadingSurface surface = GetShadingSurfaceFromHit(vertex, material, g_GlobalConstants.m_SamplerIndex_Linear_Wrap, INDIRECT_MIP_LEVEL);

    payload.m_Hit = true;
    payload.m_HitPosition = surface.m_Position;
    payload.m_HitNormal = surface.m_Normal;
    payload.m_Depth = max(0, (int) payload.m_Depth - 1);
    payload.m_Radiance = 0;

    if (payload.m_IsShadowRay)
        return;

    const RayPayload shadowRay = TraceShadowRay(surface);
    const float3 wo = normalize(-WorldRayDirection());
    const float3 wi = normalize(g_GlobalConstants.m_SunDirection.xyz);

    const float3 direct = ComputeRadiance(surface, shadowRay.m_Radiance, wi, wo);
    const float3 indirect = 0;// TraceRecursively(surface.m_Position, wo, surface.m_Normal, surface.m_Albedo, surface.m_Roughness, surface.m_Metalness, payload.m_Depth - 1);
    payload.m_Radiance = surface.m_Emission + direct + indirect;
}
