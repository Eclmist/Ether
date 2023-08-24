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

#include "utils/sampling.hlsl"
#include "utils/raytracing.hlsl"
#include "utils/encoding.hlsl"
#include "common/globalconstants.h"
#include "common/raytracingconstants.h"
#include "common/material.h"
#include "lighting/brdf.hlsl"

#define EMISSION_SCALE   400000
#define SUNLIGHT_SCALE   120000
#define SKYLIGHT_SCALE   2000
#define POINTLIGHT_SCALE 0

ConstantBuffer<GlobalConstants> g_GlobalConstants           : register(b0);
RaytracingAccelerationStructure g_RaytracingTlas            : register(t0);
StructuredBuffer<GeometryInfo> g_GeometryInfo               : register(t1);
StructuredBuffer<Material> g_MaterialTable                  : register(t2);
Texture2D<float4> g_GBufferOutput0                          : register(t3);
Texture2D<float4> g_GBufferOutput1                          : register(t4);
Texture2D<float4> g_GBufferOutput2                          : register(t5);
Texture2D<float4> g_GBufferOutput3                          : register(t6);
RWTexture2D<float4> g_LightingOutput                        : register(u0);

/* ReSTIR GI Implementation */
RWStructuredBuffer<Reservoir> g_ReSTIR_InitialReservoir     : register(u1);
RWStructuredBuffer<Reservoir> g_ReSTIR_TemporalReservoir    : register(u2);
RWStructuredBuffer<Reservoir> g_ReSTIR_SpatialReservoir     : register(u3);

float3 ComputeSkyHdri(float3 wi)
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

void SampleDirectionCosine(float3 normal, out float3 wi, out float pdf)
{
    const uint3 launchIndex = DispatchRaysIndex();
    const uint3 launchDim = DispatchRaysDimensions();
    const uint sampleIdx = launchIndex.y * launchDim.x + launchIndex.x;
    const float2 rand2D = CMJ_Sample2D(sampleIdx, 1024, 1024, g_GlobalConstants.m_FrameNumber);
    wi = TangentToWorld(SampleDirectionCosineHemisphere(rand2D), normal);
    pdf = SampleDirectionCosineHemisphere_Pdf(saturate(dot(wi, normal)));
}

void SampleDirectionUniform(float3 normal, out float3 wi, out float pdf)
{
    const uint3 launchIndex = DispatchRaysIndex();
    const uint3 launchDim = DispatchRaysDimensions();
    const uint sampleIdx = launchIndex.y * launchDim.x + launchIndex.x;
    const float2 rand2D = CMJ_Sample2D(sampleIdx, 1024, 1024, g_GlobalConstants.m_FrameNumber);
    wi = TangentToWorld(SampleDirectionHemisphere(rand2D), normal);
    pdf = SampleDirectionHemisphere_Pdf();
}

void SampleDirectionBrdf(float3 normal, float3 wo, float roughness, float metalness, out float3 wi, out float pdf)
{
    const uint3 launchIndex = DispatchRaysIndex();
    const uint3 launchDim = DispatchRaysDimensions();
    const uint sampleIdx = launchIndex.y * launchDim.x + launchIndex.x;
    const float2 rand2D = CMJ_Sample2D(sampleIdx, 1024, 1024, g_GlobalConstants.m_FrameNumber);
    const float diffuseWeight = lerp(lerp(0.5, 1.0, roughness), 0.0, metalness);
    const float specularWeight = 1.0 - diffuseWeight;
    if (Random(rand2D.x) <= specularWeight)
        wi = ImportanceSampleGGX(rand2D, wo, normal, roughness);
    else
        wi = TangentToWorld(SampleDirectionCosineHemisphere(rand2D), normal);

    const float3 H = normalize(wi + wo);
    const float nDotH = saturate(dot(normal, H));
    const float vDotH = saturate(dot(wo, H));
    const float cosTheta = saturate(dot(wi, normal));
    pdf = 1;
}

float GetTargetPdf(ReservoirSample rs)
{
    return dot(1, rs.m_Radiance);
}

bool AreDepthSimilar(float3 a, float3 b)
{
    const float aDepth = length(a - g_GlobalConstants.m_EyePosition.xyz);
    const float bDepth = length(b - g_GlobalConstants.m_EyePosition.xyz);
    return abs(aDepth - bDepth) < 50;
}

bool AreSamplesSimilar(ReservoirSample a, ReservoirSample b)
{
    //if (dot(a.m_VisibleNormal, b.m_VisibleNormal) < 0)
    //    return false;

    if (!AreDepthSimilar(a.m_VisiblePosition, b.m_VisiblePosition))
        return false;

    return true;
}

float3 ClampRadiance(float3 radiance)
{
    return min(10000, radiance);
}

bool TraceVisibility(float3 a, float3 b)
{
    RayPayload payload;
    payload.m_IsShadowRay = true;
    payload.m_Depth = 0;
    payload.m_Hit = false;

    RayDesc ray;
    ray.Direction = normalize(b - a);
    ray.Origin = a;
    ray.TMax = length(b - a) - 0.01;
    ray.TMin = 0.01;
    TraceRay(g_RaytracingTlas, RAY_FLAG_FORCE_OPAQUE, 0xFF, 0, 0, 0, ray, payload);

    return !payload.m_Hit;
}

void TraceHitSurface(
    float3 position,
    float3 direction,
    uint depth,
    out float3 hitPosition,
    out float3 hitNormal,
    out float3 radiance)
{
    RayPayload payload;
    payload.m_IsShadowRay = false;
    payload.m_Depth = depth;

    RayDesc ray;
    ray.Direction = direction;
    ray.Origin = position;
    ray.TMax = 128;
    ray.TMin = 0.01;
    TraceRay(g_RaytracingTlas, RAY_FLAG_FORCE_OPAQUE, 0xFF, 0, 0, 0, ray, payload);

    hitPosition = payload.m_HitPosition;
    hitNormal = payload.m_HitNormal;
    radiance = payload.m_Radiance;
}

float3 GetDirectRadiance(float3 position, float3 wo, float3 normal, float3 albedo, float roughness, float metalness)
{
    RayPayload payload;
    payload.m_IsShadowRay = true;

    RayDesc shadowRay;
    shadowRay.Direction = normalize(g_GlobalConstants.m_SunDirection).xyz;
    shadowRay.Origin = position + (normal * 0.01);
    shadowRay.TMax = 128;
    shadowRay.TMin = 0.01;
    TraceRay(
        g_RaytracingTlas,
        RAY_FLAG_FORCE_OPAQUE | RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH,
        0xFF,
        0,
        0,
        0,
        shadowRay,
        payload);

    const float3 pointLightColor = float3(238 / 255.0, 129 / 255.0, 57 / 255.0) * POINTLIGHT_SCALE;
    const float3 pointLightPositions[4] = {
        float3(2.1315, 2.2852, -4.665),
        float3(-5.8484, 2.2852, -4.665),
        float3(2.1315, 2.2852, 4.665),
        float3(-5.8484, 2.2852, 4.665),
    };

    float3 pointLightContribution = 0;

    for (int i = 0; i < 3; ++i)
    {
        const float3 wi = pointLightPositions[i] - float3(0.2, 0., 0) - position;
        pointLightContribution += (pointLightColor / pow(length(wi), 2.8)) *
                                  BRDF_UE4(normalize(wi), wo, normal, albedo, roughness, metalness) *
                                  saturate(dot(wi, normal));
    }

    const float3 wi = normalize(shadowRay.Direction);
    const float3 Li = payload.m_Radiance;
    const float3 f = BRDF_UE4(wi, wo, normal, albedo, roughness, metalness);
    const float cosTheta = saturate(dot(wi, normal));
    const float3 Lo = Li * f * cosTheta;

    return Lo + pointLightContribution;
}

#define IMPORTANCE_SAMPLING 0

float3 GetIndirectRadiance(
    float3 position,
    float3 wo,
    float3 normal,
    float3 albedo,
    float roughness,
    float metalness,
    uint depth)
{
    const uint3 launchIndex = DispatchRaysIndex();
    const uint3 launchDim = DispatchRaysDimensions();
    const uint sampleIdx = launchIndex.y * launchDim.x + launchIndex.x;
    const float2 rand2D = CMJ_Sample2D(sampleIdx, 1024, 1024, g_GlobalConstants.m_FrameNumber);

#if IMPORTANCE_SAMPLING == 0
    const float diffuseWeight = lerp(lerp(0.5, 1.0, roughness), 0.0, metalness);
    const float specularWeight = 1.0 - diffuseWeight;
    float3 wi = 0;
    if (Random(rand2D.x) <= specularWeight)
        wi = ImportanceSampleGGX(rand2D, wo, normal, roughness);
    else
        wi = TangentToWorld(SampleDirectionCosineHemisphere(rand2D), normal);

    const float3 H = normalize(wi + wo);
    const float nDotH = saturate(dot(normal, H));
    const float nDotV = saturate(dot(normal, wo));
    const float vDotH = saturate(dot(wo, H));
    const float cosTheta = saturate(dot(wi, normal));
    const float pdf = UE4JointPdf(specularWeight, nDotH, cosTheta, vDotH, roughness);
    const float3 f = BRDF_UE4(wi, wo, normal, albedo, roughness, metalness) / ZERO_GUARD(pdf);

#elif IMPORTANCE_SAMPLING == 1
    float3 wi = TangentToWorld(SampleDirectionCosineHemisphere(rand2D), normal);
    const float cosTheta = saturate(dot(wi, normal));
    const float pdf = cosTheta / Pi;
    const float3 f = BRDF_UE4(wi, wo, normal, albedo, roughness, metalness) / ZERO_GUARD(pdf);
#else
    float3 wi = TangentToWorld(SampleDirectionHemisphere(rand2D), normal);
    const float cosTheta = saturate(dot(wi, normal));
    const float pdf = 1 / Pi2;
    const float3 f = BRDF_UE4(wi, wo, normal, albedo, roughness, metalness) / ZERO_GUARD(pdf);
#endif

    if (depth <= 0)
        return ComputeSkyHdri(wi) * g_GlobalConstants.m_RaytracedAOIntensity;

    RayPayload payload;
    payload.m_IsShadowRay = false;
    payload.m_Depth = depth;

    RayDesc indirectRay;
    indirectRay.Direction = wi;
    indirectRay.Origin = position + (normal * 0.01);
    indirectRay.TMax = 128;
    indirectRay.TMin = 0.01;
    TraceRay(g_RaytracingTlas, RAY_FLAG_FORCE_OPAQUE, 0xFF, 0, 0, 0, indirectRay, payload);
    float3 Li = payload.m_Radiance;
    const float3 Lo = Li * f * cosTheta;
    return Lo;
}

float3 PathTrace(in MeshVertex hitSurface, in Material material, in RayPayload payload)
{
    sampler linearSampler = SamplerDescriptorHeap[g_GlobalConstants.m_SamplerIndex_Linear_Wrap];
    const uint mipLevelToSample = 8;

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
        roughness = 1 - roughnessTex.SampleLevel(linearSampler, hitSurface.m_TexCoord, mipLevelToSample).g;
    }

    if (material.m_MetalnessTextureIndex != 0)
    {
        Texture2D<float4> metalnessTex = ResourceDescriptorHeap[material.m_MetalnessTextureIndex];

        metalness = metalnessTex.SampleLevel(linearSampler, hitSurface.m_TexCoord, mipLevelToSample).b;
    }

    if (material.m_EmissiveTextureIndex != 0)
    {
        Texture2D<float4> emissiveTex = ResourceDescriptorHeap[material.m_EmissiveTextureIndex];
        emission *= emissiveTex.SampleLevel(linearSampler, hitSurface.m_TexCoord, mipLevelToSample);
    }

    // Flip normal if backface. This fixes a lot of light leakage
    normal = dot(WorldRayDirection(), hitSurface.m_Normal) > 0 ? -normal : normal;

    const float3 wo = normalize(-WorldRayDirection());
    const float3 direct = GetDirectRadiance(hitSurface.m_Position, wo, normal, albedo.xyz, roughness, metalness);
    const float3 indirectSpecular = GetIndirectRadiance(
        hitSurface.m_Position,
        wo,
        normal,
        albedo.xyz,
        roughness,
        metalness,
        payload.m_Depth - 1);

    return emission.xyz + direct + indirectSpecular;
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

    const float3 albedo = gbuffer0.xyz;
    const float3 emission = gbuffer3.xyz * EMISSION_SCALE;
    const float3 position = gbuffer1.xyz;
    const float3 normal = normalize(DecodeNormals(gbuffer2.xy));
    const float2 velocity = gbuffer2.zw;
    const float metalness = gbuffer0.w;
    const float roughness = gbuffer1.w;
    const float3 wo = normalize(g_GlobalConstants.m_EyePosition.xyz - position);

    const float2 uv = float2(launchIndex.xy) / launchDim.xy + 0.5 / launchDim.xy;
    const float2 uvPrev = clamp(uv - velocity, 0, 1);
    const uint2 launchIndexPrev = uvPrev * launchDim.xy;
    const uint sampleIdxPrev = launchIndexPrev.y * launchDim.x + launchIndexPrev.x;

    const uint depth = 1;
    float3 wi;
    float sourcePdf;

    SampleDirectionUniform(normal, wi, sourcePdf);
    // SampleDirectionCosine(normal, wi, sourcePdf);
    //SampleDirectionBrdf(normal, wo, roughness, metalness, wi, sourcePdf);
    float3 hitPosition, hitNormal, radiance;

    TraceHitSurface(position, wi, depth, hitPosition, hitNormal, radiance);

    ReservoirSample initialSample;
    initialSample.m_VisiblePosition = position;
    initialSample.m_VisibleNormal = normal;
    initialSample.m_SamplePosition = hitPosition;
    initialSample.m_SampleNormal = hitNormal;
    initialSample.m_Radiance = ClampRadiance(radiance);
    initialSample.m_Random = 0;

    Reservoir Ri;
    Ri.m_Sample = initialSample;
    Ri.m_TotalW = GetTargetPdf(initialSample);
    Ri.m_M = 1;
    Ri.m_W = Ri.m_TotalW / max(0.001, Ri.m_M * GetTargetPdf(Ri.m_Sample));
    DeviceMemoryBarrier();
  
    Reservoir Rs = g_ReSTIR_SpatialReservoir[sampleIdx];
    Reservoir Rt = g_ReSTIR_TemporalReservoir[sampleIdxPrev];

    //if (!AreDepthSimilar(Rt.m_Sample.m_VisiblePosition, position) || 
    //    length(Rt.m_Sample.m_VisiblePosition - position) > 0.1)
    //{
    //    Rt.Reset();
    //    Rs.Reset();
    //}

    //if (g_GlobalConstants.m_FrameSinceLastMovement == g_GlobalConstants.m_FrameNumber)
    //{
    //    Rt.Reset();
    //    Rs.Reset();
    //}

    //// Temporal Resampling
    const float w = GetTargetPdf(initialSample) / sourcePdf;
    Rt.Update(initialSample, w, Random(sampleIdx * g_GlobalConstants.m_FrameNumber), 30);
    Rt.m_W = Rt.m_TotalW / max(0.001, Rt.m_M * GetTargetPdf(Rt.m_Sample));
    g_ReSTIR_TemporalReservoir[sampleIdx] = Rt;
    DeviceMemoryBarrier();

    // Spatial Resampling

    Rs.m_Sample = Rt.m_Sample;
    Rs.m_TotalW = GetTargetPdf(Rt.m_Sample);
    Rs.m_M = 1;
    Rs.m_W = Rs.m_TotalW / max(0.001, Rs.m_M * GetTargetPdf(Rs.m_Sample));

    const uint numSpatialIterations = clamp(lerp(10, 3, (Rt.m_M / 30.0f)), 3, 10);
    const float searchRadius = 10;


    for (uint i = 0; i < numSpatialIterations; ++i)
    {
        const float2 rand2D = CMJ_Sample2D(sampleIdx, 1024, 1024, g_GlobalConstants.m_FrameNumber * i);
        const uint3 neighbourCoords = clamp((launchIndex + float3((rand2D * 2 - 1) * searchRadius, 0)), 0, launchDim - 1);
        const uint neighbourIdx = neighbourCoords.y * launchDim.x + neighbourCoords.x;

        Reservoir Rn = g_ReSTIR_TemporalReservoir[neighbourIdx];
        float targetPdfRn = GetTargetPdf(Rn.m_Sample);

        if (!AreSamplesSimilar(Rn.m_Sample, Rt.m_Sample))
            continue;

        if (!TraceVisibility(Rn.m_Sample.m_SamplePosition, Rt.m_Sample.m_VisiblePosition))
            targetPdfRn = 0;

        Rs.Merge(Rn, targetPdfRn, Random(sampleIdx * g_GlobalConstants.m_FrameNumber * i * 2), 100);
        Rs.m_W = Rs.m_TotalW / max(0.001, Rs.m_M * GetTargetPdf(Rs.m_Sample));
        // neighbours[i] = Rn.m_Sample;
    }

    // float Z = 0;

    // for (uint i = 0; i < numSpatialIterations; ++i)
    //{
    //     if (GetTargetPdf(neighbours[i].m_Sample) > 0)
    //         Z += R
    //
    // }

    g_ReSTIR_SpatialReservoir[sampleIdx] = Rs;
    DeviceMemoryBarrier();



    Reservoir finalReservoir = Rs;
    //if (uv.x + uv.y / 10 > sin(g_GlobalConstants.m_Time.y) * 0.7 + 0.7)
    //    finalReservoir = Ri;

    ReservoirSample finalSample = finalReservoir.m_Sample;
    const float3 finalWo = wo;
    const float3 finalWi = normalize(finalSample.m_SamplePosition - finalSample.m_VisiblePosition);

    const float3 f = albedo;//BRDF_UE4(finalWi, finalWo, normal, albedo, roughness, metalness);
    const float3 indirect = finalSample.m_Radiance * finalReservoir.m_W * f * saturate(dot(normal, finalWi));

    const float3 direct = GetDirectRadiance(position, wo, normal, albedo, roughness, metalness);
    g_LightingOutput[launchIndex.xy].xyz = emission + direct + indirect * 3;
}

[shader("miss")]
void Miss(inout RayPayload payload)
{
    payload.m_Hit = false;
    payload.m_Radiance = 0;

    if (payload.m_IsShadowRay)
    {
        // Sample sun color
        payload.m_Radiance = g_GlobalConstants.m_SunColor.xyz *
                             lerp(
                                 400.0f,
                                 SUNLIGHT_SCALE,
                                 saturate(dot(g_GlobalConstants.m_SunDirection.xyz, float3(0, 1, 0))));
    }
    else
    {
        // Sample sky color
        payload.m_Radiance = ComputeSkyHdri(WorldRayDirection());
    }
}

[shader("closesthit")]
void ClosestHit(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attribs)
{
    const GeometryInfo geoInfo = g_GeometryInfo[InstanceIndex()];
    const MeshVertex vertex = GetHitSurface(attribs, geoInfo);
    const Material material = g_MaterialTable[geoInfo.m_MaterialIndex];

    payload.m_Hit = true;
    payload.m_Radiance = 0;
    payload.m_HitPosition = vertex.m_Position;
    payload.m_HitNormal = vertex.m_Normal;

    if (!payload.m_IsShadowRay)
        payload.m_Radiance = PathTrace(vertex, material, payload);
}
