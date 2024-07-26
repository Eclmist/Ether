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
#define USE_IMPORTANCE_SAMPLING     1
#define USE_BRDF_TARGET_FUNCTION    1
#define USE_JACOBIAN                0
#define USE_WEIRD_HISTORY_CLAMP     1

#define TEMPORAL_HISTORY            30
#define SPATIAL_HISTORY             500
#define SPATIAL_RADIUS              20

// TODO: Make lightingcommon.h
#define EMISSION_SCALE              10000
#define SUNLIGHT_SCALE              120000
#define SKYLIGHT_SCALE              10000
#define POINTLIGHT_SCALE            2000
#define MAX_BOUNCES                 0

#define ZERO_GUARD(f) (max(0.001, f))

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

bool TraceVisibility(float3 a, float3 b)
{
    RayPayload payload;
    payload.m_IsShadowRay = true;
    payload.m_Depth = 1;
    payload.m_Hit = false;

    RayDesc ray;
    ray.Direction = normalize(b - a);
    ray.Origin = a;
    ray.TMax = length(b - a) - 0.01;
    ray.TMin = 0.01;
    TraceRay(g_RaytracingTlas, RAY_FLAG_FORCE_OPAQUE, 0xFF, 0, 0, 0, ray, payload);

    return !payload.m_Hit;
}

float3 ComputeSkyHdri(float3 wi)
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
        wi = normalize(TangentToWorld(SampleDirectionCosineHemisphere(rand2D), normal));

    const float3 H = normalize(wi + wo);
    const float nDotH = saturate(dot(normal, H));
    const float nDotV = saturate(dot(normal, wo));
    const float vDotH = saturate(dot(wo, H));

    // TODO: Abs is also wrong here. Why does it work?
    const float cosTheta = abs(dot(-wi, normal));

    if (importanceSampleBrdf)
        pdf = UE4JointPdf(specularWeight, nDotH, cosTheta, vDotH, roughness);
    else
        pdf = SampleDirectionHemisphere_Pdf();
}

float TargetFunction(ReservoirSample rs)
{
    const float3 normal = normalize(rs.m_VisibleNormal);
    const float3 wi = normalize(rs.m_SamplePosition - rs.m_VisiblePosition);
    const float3 f = BRDF_UE4(rs.m_Wi, rs.m_Wo, rs.m_VisibleNormal, rs.m_Albedo, rs.m_Roughness, rs.m_Metalness);

    //const float cosTheta = saturate(dot(normal, wi));
    // const float3 H = normalize(rs.m_Wi + rs.m_Wo);
    // const float nDotH = saturate(dot(rs.m_VisibleNormal, H));
    // const float vDotH = saturate(dot(rs.m_Wo, H));
    // const float diffuseWeight = lerp(lerp(0.5, 1.0, rs.m_Roughness), 0.0, rs.m_Metalness);
    // const float specularWeight = 1.0 - diffuseWeight;
    // const float pdf = UE4JointPdf(specularWeight, nDotH, cosTheta, vDotH, rs.m_Roughness);

#if USE_BRDF_TARGET_FUNCTION
    return dot(1, rs.m_Radiance * f);
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

float ComputeJacobianDeterminant(float3 newPoint, float3 oldPoint, float3 sharedPoint, float3 sharedNormal)
{
    float3 oldPath = oldPoint - sharedPoint; 
    float3 newPath = newPoint - sharedPoint; 
    float oldLengthSqr = ZERO_GUARD(abs(dot(oldPath, oldPath)));
    float newLengthSqr = ZERO_GUARD(abs(dot(newPath, newPath)));
    float oldCosAngle = ZERO_GUARD(saturate(dot(normalize(oldPath), sharedNormal)));
    float newCosAngle = ZERO_GUARD(saturate(dot(normalize(newPath), sharedNormal)));

    //return ZERO_GUARD(newCosAngle * oldLengthSqr) / ZERO_GUARD(oldCosAngle * newLengthSqr);
    return (newCosAngle / oldCosAngle) * (oldLengthSqr / newLengthSqr);
}

void PathTrace(in MeshVertex hitSurface, in Material material, inout RayPayload payload)
{
    sampler linearSampler = SamplerDescriptorHeap[g_GlobalConstants.m_SamplerIndex_Linear_Wrap];
    const uint mipLevelToSample = 8;

    float3 albedo = material.m_BaseColor.rgb;
    float3 emission = material.m_EmissiveColor.rgb * EMISSION_SCALE;
    float3 normal = hitSurface.m_Normal;
    float roughness = 1;
    float metalness = 0;

    if (material.m_AlbedoTextureIndex != 0)
    {
        Texture2D<float4> albedoTex = ResourceDescriptorHeap[material.m_AlbedoTextureIndex];
        albedo *= albedoTex.SampleLevel(linearSampler, hitSurface.m_TexCoord, mipLevelToSample).rgb;
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
        emission *= emissiveTex.SampleLevel(linearSampler, hitSurface.m_TexCoord, mipLevelToSample).rgb;
    }

    // Flip normal if backface. This fixes a lot of light leakage
    normal = dot(WorldRayDirection(), hitSurface.m_Normal) > 0 ? -normal : normal;
    const float3 wo = normalize(-WorldRayDirection());
    float3 Lo_direct = 0;
    float3 Lo_indirect = 0;

    payload.m_HitPosition = hitSurface.m_Position;
    payload.m_HitNormal = normal;

    // Direct / shadow ray
    {
        RayPayload shadowPayload;
        shadowPayload.m_IsShadowRay = true;

        RayDesc shadowRay;
        shadowRay.Direction = normalize(g_GlobalConstants.m_SunDirection).xyz;
        shadowRay.Origin = hitSurface.m_Position + (normal * 0.01);
        shadowRay.TMax = 128;
        shadowRay.TMin = 0.01;
        TraceRay(g_RaytracingTlas, RAY_FLAG_FORCE_OPAQUE | RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH, 0xFF, 0, 0, 0, shadowRay, shadowPayload);

        const float3 wi = normalize(shadowRay.Direction);
        const float3 Li = shadowPayload.m_Radiance;
        const float3 f = BRDF_UE4(wi, wo, normal, albedo, roughness, metalness);
        const float cosTheta = saturate(dot(wi, normal));
        Lo_direct = Li * f * cosTheta;
    }

    // Indirect / recursive ray
    if (payload.m_Depth < MAX_BOUNCES)
    {
        const uint3 launchIndex = DispatchRaysIndex();
        const uint3 launchDim = DispatchRaysDimensions();
        const uint sampleIdx = launchIndex.y * launchDim.x + launchIndex.x;
        const float2 rand2D = CMJ_Sample2D(sampleIdx, 1024, 1024, g_GlobalConstants.m_FrameNumber);

// 0 -> Importance sample BRDF
// 1 -> Importance sample Cosine Hemisphere
// 2 -> Importance sample Hemisphere
#define IMPORTANCE_SAMPLING 2

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

        RayPayload indirectPayload;
        indirectPayload.m_IsShadowRay = false;
        indirectPayload.m_Depth = payload.m_Depth + 1;

        RayDesc indirectRay;
        indirectRay.Direction = wi;
        indirectRay.Origin = hitSurface.m_Position;
        indirectRay.TMax = 128;
        indirectRay.TMin = 0.01;
        TraceRay(g_RaytracingTlas, RAY_FLAG_FORCE_OPAQUE, 0xFF, 0, 0, 0, indirectRay, indirectPayload);
        Lo_indirect = indirectPayload.m_Radiance * f * cosTheta;
    }

    payload.m_Radiance = emission.xyz + Lo_direct + Lo_indirect;
}

void TraceInitialSample(
    float3 position,
    float3 direction,
    uint depth,
    out float3 hitPosition,
    out float3 hitNormal,
    out float3 radiance)
{
    RayPayload payload;
    payload.m_IsShadowRay = false;
    payload.m_Depth = 0;
    payload.m_Hit = false;
    payload.m_Radiance = 0;
    payload.m_HitPosition = 0;
    payload.m_HitNormal = 0;

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

    radiance = payload.m_Radiance;
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

#if USE_IMPORTANCE_SAMPLING
    SampleDirectionBrdf(normal, wo, roughness, metalness, wi, proposalPdf);
#else
    SampleDirectionUniform(normal, wi, proposalPdf);
#endif

    float3 hitPosition, hitNormal, radiance;
    TraceInitialSample(position, wi, MAX_BOUNCES, hitPosition, hitNormal, radiance);

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

    Reservoir Ri;
    Ri.m_Sample = initialSample;
    Ri.m_TotalWeight = TargetFunction(initialSample) / ZERO_GUARD(proposalPdf);
    Ri.m_NumSamples = 1;
    Ri.m_ContributionWeight = Ri.m_TotalWeight / ZERO_GUARD(Ri.m_NumSamples * TargetFunction(Ri.m_Sample));

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
    DeviceMemoryBarrier();

    if (!AreSamplesSimilar(Rt.m_Sample, Ri.m_Sample))
        Rt.Reset();

    if (launchIndexPrev.x < 0 || launchIndexPrev.x >= launchDim.x || 
        launchIndexPrev.y < 0 || launchIndexPrev.y >= launchDim.y)
        Rt.Reset();

    // Reset on move (DEBUG)
    //if (g_GlobalConstants.m_FrameNumber == g_GlobalConstants.m_FrameSinceLastMovement)
    //     Rt.Reset();

    const float rand = Random(sampleIdx * g_GlobalConstants.m_FrameNumber);

#if USE_TEMPORAL_RESAMPLING

#if USE_WEIRD_HISTORY_CLAMP
    Ri.Merge(Rt, TargetFunction(Rt.m_Sample), rand, TEMPORAL_HISTORY);
#else
    Rt.Update(Ri.m_Sample, Ri.m_TotalWeight, rand);
    Ri = Rt;
#endif

    Ri.m_ContributionWeight = Ri.m_TotalWeight / ZERO_GUARD(Ri.m_NumSamples * TargetFunction(Ri.m_Sample));
#endif

    g_ReSTIR_StagingReservoir[sampleIdx] = Ri;
}

void ApplySpatialResampling()
{
    
    const uint3 launchIndex = DispatchRaysIndex();
    const uint3 launchDim = DispatchRaysDimensions();
    const uint sampleIdx = launchIndex.y * launchDim.x + launchIndex.x;

    Reservoir Rt = g_ReSTIR_StagingReservoir[sampleIdx];
    Reservoir Rs = Rt;

    const float3 position = Rs.m_Sample.m_VisiblePosition;
    const float3 normal = Rs.m_Sample.m_VisibleNormal;
    const float viewDepth = length(position - g_GlobalConstants.m_CameraPosition.xyz);

    //const int numSpatialIterations = Rs.m_NumSamples < 30 ? 10 : 3;
    const int numSpatialIterations = 10;

    for (uint i = 0; i < numSpatialIterations; ++i)
    {
        const float2 rand2D = CMJ_Sample2D(sampleIdx, 1024, 1024, g_GlobalConstants.m_FrameNumber * i);
        const int2 neighbourCoords = (launchIndex.xy + 0.5f) + SquareToConcentricDiskMapping(rand2D) * (SPATIAL_RADIUS / numSpatialIterations * i);
        const uint neighbourIdx = clamp(neighbourCoords.y * launchDim.x + neighbourCoords.x, 0, launchDim.x * launchDim.y);
        Reservoir neighbour = g_ReSTIR_StagingReservoir[neighbourIdx];
        ReservoirSample nSample = neighbour.m_Sample;

        if (neighbourCoords.x < 0 || neighbourCoords.x >= launchDim.x ||
            neighbourCoords.y < 0 || neighbourCoords.y >= launchDim.y)
            continue;

        const float3 RnPosition = nSample.m_VisiblePosition;
        const float3 RnNormal = nSample.m_VisibleNormal;
        const float RnViewDepth = length(RnPosition - g_GlobalConstants.m_CameraPosition.xyz);

        if (dot(RnNormal, normal) < 0.5)
            continue;
        if (abs(viewDepth - RnViewDepth) > 0.5)
            continue;

#if USE_JACOBIAN
        float jacobian = ComputeJacobianDeterminant(position, nSample.m_VisiblePosition, nSample.m_SamplePosition, nSample.m_SampleNormal);
        float targetFunction = TargetFunction(neighbour.m_Sample) / ZERO_GUARD(jacobian);
#else
        float targetFunction = TargetFunction(neighbour.m_Sample);
#endif

        //if (!TraceVisibility(nSample.m_SamplePosition, Rs.m_Sample.m_VisiblePosition))
        //    targetFunction = 0;

        const float rand = Random(sampleIdx * g_GlobalConstants.m_FrameNumber);
        Rs.Merge(neighbour, targetFunction, rand, SPATIAL_HISTORY);
    }

    Rs.m_ContributionWeight = Rs.m_TotalWeight / ZERO_GUARD(Rs.m_NumSamples * TargetFunction(Rs.m_Sample));

    {
    
    }
    const float4 gbuffer0 = g_GBufferOutput0.Load(launchIndex);
    const float4 gbuffer1 = g_GBufferOutput1.Load(launchIndex);
    const float4 gbuffer2 = g_GBufferOutput2.Load(launchIndex);
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

    float3 Lo_direct = 0;
    float3 Lo_indirect = 0;

    {   // Direct / shadow ray
        RayPayload payload;
        payload.m_IsShadowRay = true;

        RayDesc shadowRay;
        shadowRay.Direction = normalize(g_GlobalConstants.m_SunDirection).xyz;
        shadowRay.Origin = gbuffer1.xyz + (DecodeNormals(gbuffer2.xy) * 0.01);
        shadowRay.TMax = 128;
        shadowRay.TMin = 0.01;
        TraceRay(g_RaytracingTlas, RAY_FLAG_FORCE_OPAQUE | RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH, 0xFF, 0, 0, 0, shadowRay, payload);

        const float3 wi = normalize(shadowRay.Direction);
        const float3 Li = payload.m_Radiance;
        const float3 f = BRDF_UE4(wi, wo, normal, albedo, roughness, metalness);
        const float cosTheta = saturate(dot(wi, normal));
        Lo_direct = Li * f * cosTheta;
    }

    Lo_indirect = finalSample.m_Radiance * finalReservoir.m_ContributionWeight * f * saturate(dot(normalize(finalSample.m_Wi), normal));
    g_LightingOutput[launchIndex.xy].xyz = emission + Lo_direct + Lo_indirect;

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

        PathTrace(vertex, material, payload);
    }
}
