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

#define USE_TEMPORAL_RESAMPLING     1
#define USE_SPATIAL_RESAMPLING      1
#define USE_SPATIAL_ACCUMULATION    0
#define USE_IMPORTANCE_SAMPLING     0
#define USE_BRDF_TARGET_FUNCTION    1

#define TEMPORAL_HISTORY            30
#define SPATIAL_HISTORY             500
#define SPATIAL_RADIUS              20

// TODO: Make lightingcommon.h
#define EMISSION_SCALE              100000
#define SUNLIGHT_SCALE              120000
#define SKYLIGHT_SCALE              0
#define POINTLIGHT_SCALE            2000
#define MAX_RAY_DEPTH               1

struct RootConstants
{
    uint m_PassIndex;
};

ConstantBuffer<GlobalConstants> g_GlobalConstants       : register(b0);
ConstantBuffer<RootConstants> g_RootConstants           : register(b1);

RaytracingAccelerationStructure g_RaytracingTlas        : register(t0);
StructuredBuffer<GeometryInfo> g_GeometryInfo           : register(t1);
StructuredBuffer<Material> g_MaterialTable              : register(t2);
Texture2D<float4> g_GBufferOutput0                      : register(t3);
Texture2D<float4> g_GBufferOutput1                      : register(t4);
Texture2D<float4> g_GBufferOutput2                      : register(t5);
Texture2D<float4> g_GBufferOutput3                      : register(t6);
RWTexture2D<float4> g_LightingOutput                    : register(u0);

/* ReSTIR GI Implementation */
RWStructuredBuffer<Reservoir> g_ReSTIR_GIReservoir      : register(u1);
RWStructuredBuffer<Reservoir> g_ReSTIR_StagingReservoir : register(u2);

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

void SampleDirectionBrdf(
    float3 normal,
    float3 wo,
    float roughness,
    float metalness,
    out float3 wi,
    out float pdf)
{
    const uint3 launchIndex = DispatchRaysIndex();
    const uint3 launchDim = DispatchRaysDimensions();
    const uint sampleIdx = launchIndex.y * launchDim.x + launchIndex.x;
    const float2 rand2D = CMJ_Sample2D(sampleIdx, 1024, 1024, g_GlobalConstants.m_FrameNumber);

    const float diffuseWeight = lerp(lerp(0.5, 1.0, roughness), 0.0, metalness);
    const float specularWeight = 1.0 - diffuseWeight;

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

    if (importanceSampleBrdf)
        pdf = UE4JointPdf(specularWeight, nDotH, cosTheta, vDotH, roughness);
    else
        pdf = SampleDirectionHemisphere_Pdf();
}

float TargetPdf(ReservoirSample rs)
{
    const float3 normal = rs.m_VisibleNormal;
    const float3 wi = normalize(rs.m_SamplePosition - rs.m_VisiblePosition);
    const float3 f = BRDF_UE4(rs.m_Wi, rs.m_Wo, rs.m_VisibleNormal, rs.m_Albedo, rs.m_Roughness, rs.m_Metalness);

    const float cosTheta = saturate(dot(normal, wi));
    // const float3 H = normalize(rs.m_Wi + rs.m_Wo);
    // const float nDotH = saturate(dot(rs.m_VisibleNormal, H));
    // const float vDotH = saturate(dot(rs.m_Wo, H));
    // const float diffuseWeight = lerp(lerp(0.5, 1.0, rs.m_Roughness), 0.0, rs.m_Metalness);
    // const float specularWeight = 1.0 - diffuseWeight;
    // const float pdf = UE4JointPdf(specularWeight, nDotH, cosTheta, vDotH, rs.m_Roughness);

#if USE_BRDF_TARGET_FUNCTION
    return dot(1, rs.m_Radiance) * f;
#else
    return dot(1, rs.m_Radiance);
#endif
}

bool AreDepthSimilar(float3 a, float3 b)
{
    const float aDepth = length(a - g_GlobalConstants.m_CameraPosition.xyz);
    const float bDepth = length(b - g_GlobalConstants.m_CameraPosition.xyz);
    return abs(aDepth - bDepth) < 0.3;
}

bool AreSamplesSimilar(ReservoirSample a, ReservoirSample b)
{
    if (dot(a.m_VisibleNormal, b.m_VisibleNormal) < 0.9)
        return false;

    if (!AreDepthSimilar(a.m_VisiblePosition, b.m_VisiblePosition))
        return false;

    return true;
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
    payload.m_Hit = false;

    RayDesc ray;
    ray.Direction = direction;
    ray.Origin = position;
    ray.TMax = 128;
    ray.TMin = 0.01;
    TraceRay(g_RaytracingTlas, RAY_FLAG_FORCE_OPAQUE, 0xFF, 0, 0, 0, ray, payload);

    if (payload.m_Hit)
    {
        hitPosition = payload.m_HitPosition;
        hitNormal = payload.m_HitNormal;
    }
    else
    {
        hitPosition = position + direction;
        hitNormal = 0;
    }

    radiance = payload.m_Radiance;
}

float3 TraceDirectLighting(float3 position, float3 wo, float3 normal, float3 albedo, float roughness, float metalness)
{
    RayPayload payload;
    payload.m_IsShadowRay = true;

    RayDesc shadowRay;
    shadowRay.Direction = normalize(g_GlobalConstants.m_SunDirection).xyz;
    shadowRay.Origin = position + (normal * 0.01);
    shadowRay.TMax = 128;
    shadowRay.TMin = 0.01;
    TraceRay(g_RaytracingTlas, RAY_FLAG_FORCE_OPAQUE | RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH, 0xFF, 0, 0, 0, shadowRay, payload);

    const float3 wi = normalize(shadowRay.Direction);
    const float3 Li = payload.m_Radiance;
    const float3 f = ZERO_GUARD(BRDF_UE4(wi, wo, normal, albedo, roughness, metalness));
    const float cosTheta = saturate(dot(wi, normal));
    const float3 Lo = Li * f * cosTheta;
    return Lo;
}

// 0 -> Importance sample BRDF
// 1 -> Importance sample Cosine Hemisphere
// 2 -> Importance sample Hemisphere
#define IMPORTANCE_SAMPLING 0

float3 TraceRecursively(
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
        return ComputeSkyHdri(wo) * 0.2f;

    RayPayload payload;
    payload.m_IsShadowRay = false;
    payload.m_Depth = depth;

    RayDesc indirectRay;
    indirectRay.Direction = wi;
    indirectRay.Origin = position;
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
    const float3 direct = TraceDirectLighting(hitSurface.m_Position, wo, normal, albedo.xyz, roughness, 0);
    const float3 indirect = TraceRecursively(hitSurface.m_Position, wo, normal, albedo.xyz, roughness, metalness, payload.m_Depth - 1);
    return emission.xyz + direct + indirect;
}

void GenerateInitialSamples()
{
    sampler linearSampler = SamplerDescriptorHeap[g_GlobalConstants.m_SamplerIndex_Linear_Clamp];
    const uint3 launchIndex = DispatchRaysIndex();
    const uint3 launchDim = DispatchRaysDimensions();
    const uint sampleIdx = launchIndex.y * launchDim.x + launchIndex.x;

    const float4 gbuffer0 = g_GBufferOutput0.Load(launchIndex);
    const float4 gbuffer1 = g_GBufferOutput1.Load(launchIndex);
    const float4 gbuffer2 = g_GBufferOutput2.Load(launchIndex);
    const float4 gbuffer3 = g_GBufferOutput3.Load(launchIndex);

    const float3 albedo = gbuffer0.rgb;
    const float3 emission = gbuffer3.rgb * EMISSION_SCALE;
    const float3 position = gbuffer1.xyz;
    const float3 viewDir = normalize(g_GlobalConstants.m_CameraPosition.xyz - position);
    const float viewDepth = length(position - g_GlobalConstants.m_CameraPosition.xyz);
    const float3 normal = normalize(DecodeNormals(gbuffer2.xy));
    const float metalness = gbuffer0.w;
    const float roughness = gbuffer1.w;
    const float3 wo = normalize(g_GlobalConstants.m_CameraPosition.xyz - position);
    float3 wi;
    float proposalPdf;
    float initialSamplePdf;

    // [NOTE]
    // Non-uniform sampling have possiblity that the PDF of the sample is very small
    // This can cause firefly. Since RIS proposal sample is not that important
    // the tradeoff is made here for SLIGHTLY worse convergence in exchange for no firefly.

#if USE_IMPORTANCE_SAMPLING
    SampleDirectionBrdf(normal, wo, roughness, metalness, wi, proposalPdf);
#else
    SampleDirectionUniform(normal, wi, proposalPdf);
#endif

    float3 hitPosition, hitNormal, radiance;
    TraceHitSurface(position, wi, MAX_RAY_DEPTH, hitPosition, hitNormal, radiance);

    ReservoirSample initialSample;
    initialSample.m_Wo = wo;
    initialSample.m_Wi = wi;
    initialSample.m_Albedo = albedo;
    initialSample.m_Roughness = roughness;
    initialSample.m_Metalness = metalness;
    initialSample.m_VisiblePosition = position;
    initialSample.m_VisibleNormal = normal;
    initialSample.m_SamplePosition = hitPosition;
    initialSample.m_SampleNormal = hitNormal;
    initialSample.m_Radiance = min(10000, radiance);
    initialSample.m_FrameNumber = g_GlobalConstants.m_FrameNumber;

    Reservoir Ri;
    Ri.m_Sample = initialSample;
    Ri.m_w = TargetPdf(initialSample) / ZERO_GUARD(proposalPdf);
    Ri.m_M = 1;
    Ri.m_W = Ri.m_w / ZERO_GUARD(Ri.m_M * TargetPdf(Ri.m_Sample));

    g_ReSTIR_StagingReservoir[sampleIdx] = Ri;
}

void ApplyTemporalResampling()
{
    const uint3 launchIndex = DispatchRaysIndex();
    const uint3 launchDim = DispatchRaysDimensions();
    const uint sampleIdx = launchIndex.y * launchDim.x + launchIndex.x;

    const float4 gbuffer2 = g_GBufferOutput2.Load(launchIndex);
    const float2 velocity = gbuffer2.zw;
    const int2 launchIndexPrev = ((float2)launchIndex.xy + 0.5) - (velocity * launchDim.xy);

    const uint sampleIdxPrev = clamp(launchIndexPrev.y * launchDim.x + launchIndexPrev.x, 0, launchDim.x * launchDim.y);

    Reservoir Ri = g_ReSTIR_StagingReservoir[sampleIdx];
    Reservoir Rt = g_ReSTIR_GIReservoir[sampleIdxPrev];

    if (!AreSamplesSimilar(Rt.m_Sample, Ri.m_Sample))
        Rt.Reset(); // DEBUG! Reprojection is broken (velocity is not 0 even when not moving? wtf?)

    if (launchIndexPrev.x < 0 || launchIndexPrev.x >= launchDim.x || 
        launchIndexPrev.y < 0 || launchIndexPrev.y >= launchDim.y)
        Rt.Reset();

    // if (g_GlobalConstants.m_FrameNumber == g_GlobalConstants.m_FrameSinceLastMovement)
    //     Rt.Reset(); // DEBUG! Reprojection is broken (velocity is not 0 even when not moving? wtf?)

    // Apparently, we don't need rand here?
    const float rand = Random(sampleIdx * g_GlobalConstants.m_FrameNumber);

#if USE_TEMPORAL_RESAMPLING
    Ri.Merge(Rt, TargetPdf(Rt.m_Sample), rand, TEMPORAL_HISTORY);
    Ri.m_W = Ri.m_w / ZERO_GUARD(Ri.m_M * TargetPdf(Ri.m_Sample));
#endif

    g_ReSTIR_StagingReservoir[sampleIdx] = Ri;
}

void ApplySpatialResampling()
{
    const uint3 launchIndex = DispatchRaysIndex();
    const uint3 launchDim = DispatchRaysDimensions();
    const uint sampleIdx = launchIndex.y * launchDim.x + launchIndex.x;

    const float4 gbuffer1 = g_GBufferOutput1.Load(launchIndex);
    const float4 gbuffer2 = g_GBufferOutput2.Load(launchIndex);

    const float3 position = gbuffer1.xyz;
    const float3 normal = normalize(DecodeNormals(gbuffer2.xy));
    const float viewDepth = length(position - g_GlobalConstants.m_CameraPosition.xyz);

    Reservoir Rt = g_ReSTIR_StagingReservoir[sampleIdx];
    Reservoir Rs = Rt;
    const int numSpatialIterations = Rs.m_M < 30 ? 10 : 3;
    //const int numSpatialIterations = 3;

    for (uint i = 0; i < numSpatialIterations; ++i)
    {
        const float2 rand2D = CMJ_Sample2D(sampleIdx, 1024, 1024, g_GlobalConstants.m_FrameNumber * i);
        const int2 neighbourCoords = (launchIndex.xy + 0.5f) + SquareToConcentricDiskMapping(rand2D) * SPATIAL_RADIUS;
        const uint neighbourIdx = clamp(neighbourCoords.y * launchDim.x + neighbourCoords.x, 0, launchDim.x * launchDim.y);
        Reservoir neighbour = g_ReSTIR_StagingReservoir[neighbourIdx];

        if (neighbourCoords.x < 0 || neighbourCoords.x >= launchDim.x ||
            neighbourCoords.y < 0 || neighbourCoords.y >= launchDim.y)
            continue;

        const float4 gbufferNeighbour1 = g_GBufferOutput1.Load(int3(neighbourCoords, 0));
        const float4 gbufferNeighbour2 = g_GBufferOutput2.Load(int3(neighbourCoords, 0));
        const float3 RnPosition = gbufferNeighbour1.xyz;
        const float RnViewDepth = length(RnPosition - g_GlobalConstants.m_CameraPosition.xyz);
        const float3 RnNormal = normalize(DecodeNormals(gbufferNeighbour2.xy));

        if (dot(RnNormal, normal) < 0.5)
            continue;
        if (abs(viewDepth - RnViewDepth) > 0.5)
            continue;

        // Apparently, we don't need rand here?
        const float rand = Random(sampleIdx * g_GlobalConstants.m_FrameNumber);
        Rs.Merge(neighbour, TargetPdf(neighbour.m_Sample), rand, SPATIAL_HISTORY);
    }

    Rs.m_W = Rs.m_w / ZERO_GUARD(Rs.m_M * TargetPdf(Rs.m_Sample));

    const float4 gbuffer0 = g_GBufferOutput0.Load(launchIndex);
    const float4 gbuffer3 = g_GBufferOutput3.Load(launchIndex);

    const float3 albedo = gbuffer0.rgb;
    const float3 wo = normalize(g_GlobalConstants.m_CameraPosition.xyz - position);
    const float3 emission = gbuffer3.xyz * EMISSION_SCALE;
    const float roughness = gbuffer1.w;
    const float metalness = gbuffer0.w;

#if USE_SPATIAL_RESAMPLING
    Reservoir finalReservoir = Rs;
#else
    Reservoir finalReservoir = Rt;
#endif

    ReservoirSample finalSample = finalReservoir.m_Sample;

    const float3 f = BRDF_UE4(finalSample.m_Wi, finalSample.m_Wo, finalSample.m_VisibleNormal, albedo, finalSample.m_Roughness, finalSample.m_Metalness);
    const float3 indirect = min(10000, finalSample.m_Radiance * finalReservoir.m_W * f * saturate(dot(finalSample.m_VisibleNormal, finalSample.m_Wi)));
    const float3 direct = TraceDirectLighting(position, wo, normal, albedo, roughness, metalness);
    finalSample.m_Radiance = indirect;

    g_LightingOutput[launchIndex.xy].xyz = emission + direct + indirect;

#if USE_SPATIAL_ACCUMULATION && USE_SPATIAL_RESAMPLING
    g_ReSTIR_GIReservoir[sampleIdx] = Rs;
#else
    g_ReSTIR_GIReservoir[sampleIdx] = Rt;
#endif
}

void EvaluateFinalLighting()
{
}

[shader("raygeneration")]
void RayGeneration()
{
    if (g_RootConstants.m_PassIndex == RESTIR_INITIAL_PASS)
        GenerateInitialSamples();
    
    if (g_RootConstants.m_PassIndex == RESTIR_TEMPORAL_PASS)
        ApplyTemporalResampling();

    if (g_RootConstants.m_PassIndex == RESTIR_SPATIAL_PASS)
        ApplySpatialResampling();
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
        payload.m_Radiance = ComputeSkyHdri(WorldRayDirection());
    }
}

[shader("closesthit")]
void ClosestHit(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attribs)
{
    payload.m_Hit = true;
    payload.m_Radiance = 0;

    if (!payload.m_IsShadowRay)
    {
        const GeometryInfo geoInfo = g_GeometryInfo[InstanceIndex()];
        const MeshVertex vertex = GetHitSurface(attribs, geoInfo);
        const Material material = g_MaterialTable[geoInfo.m_MaterialIndex];

        payload.m_Radiance = PathTrace(vertex, material, payload);
    }
}
