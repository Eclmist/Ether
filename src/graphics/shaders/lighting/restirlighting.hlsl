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
#include "utils/helpers.hlsl"
#include "common/globalconstants.h"
#include "common/raytracingconstants.h"
#include "common/material.h"
#include "lighting/brdf.hlsl"

#define USE_TEMPORAL_RESAMPLING     1
#define USE_SPATIAL_RESAMPLING      1
#define USE_SPATIAL_ACCUMULATION    0
#define USE_IMPORTANCE_SAMPLING     1
#define USE_JACOBIAN                1

#define TEMPORAL_HISTORY            30
#define SPATIAL_HISTORY             500
#define SPATIAL_RADIUS              50

#define DEBUG_GREYSCALE_OUTPUT      0
#define DEBUG_RESET_HISTORY         0

// TODO: Make lightingcommon.h
#define EMISSION_SCALE              10000
#define SUNLIGHT_SCALE              120000
#define SKYLIGHT_SCALE              10000
#define POINTLIGHT_SCALE            2000
#define MAX_BOUNCES                 0

#ifdef ZERO_GUARD
#undef ZERO_GUARD
#endif

#define ZERO_GUARD(f) (max(0.001, f))

struct GBufferSurface
{
    float3 m_Position;
    float3 m_Normal;
    float3 m_Albedo;
    float3 m_Emission;
    float m_Roughness;
    float m_Metalness;
    float2 m_Velocity;
};

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

void SampleDirectionCosine(GBufferSurface surface, out float3 wi, out float pdf)
{
    const uint3 launchIndex = DispatchRaysIndex();
    const uint3 launchDim = DispatchRaysDimensions();
    const uint sampleIdx = launchIndex.y * launchDim.x + launchIndex.x;
    const float2 rand2D = CMJ_Sample2D(sampleIdx, 1024, 1024, g_GlobalConstants.m_FrameNumber);
    wi = TangentToWorld(SampleDirectionCosineHemisphere(rand2D), surface.m_Normal);
    pdf = SampleDirectionCosineHemisphere_Pdf(saturate(dot(wi, surface.m_Normal)));
}

void SampleDirectionUniform(GBufferSurface surface, out float3 wi, out float pdf)
{
    const uint3 launchIndex = DispatchRaysIndex();
    const uint3 launchDim = DispatchRaysDimensions();
    const uint sampleIdx = launchIndex.y * launchDim.x + launchIndex.x;
    const float2 rand2D = CMJ_Sample2D(sampleIdx, 1024, 1024, g_GlobalConstants.m_FrameNumber);
    wi = TangentToWorld(SampleDirectionHemisphere(rand2D), surface.m_Normal);
    pdf = SampleDirectionHemisphere_Pdf();
}

void SampleDirectionBrdf(GBufferSurface surface, out float3 wi, out float pdf)
{
    const float3 wo = normalize(g_GlobalConstants.m_CameraPosition.xyz - surface.m_Position);

    const uint3 launchIndex = DispatchRaysIndex();
    const uint3 launchDim = DispatchRaysDimensions();
    const uint sampleIdx = launchIndex.y * launchDim.x + launchIndex.x;
    const float2 rand2D = CMJ_Sample2D(sampleIdx, 1024, 1024, g_GlobalConstants.m_FrameNumber);

    const float diffuseWeight = lerp(lerp(0.5, 1.0, surface.m_Roughness), 0.0, surface.m_Metalness);
    const float specularWeight = 1.0 - diffuseWeight;

    const bool importanceSampleBrdf = Random(rand2D.x) <= specularWeight;

    if (importanceSampleBrdf)
        wi = normalize(ImportanceSampleGGX(rand2D, wo, surface.m_Normal, surface.m_Roughness));
    else
        wi = normalize(TangentToWorld(SampleDirectionCosineHemisphere(rand2D), surface.m_Normal));

    const float3 H = normalize(wi + wo);
    const float nDotH = saturate(dot(surface.m_Normal, H));
    const float nDotV = saturate(dot(surface.m_Normal, wo));
    const float vDotH = saturate(dot(wo, H));

    // TODO: Abs is also wrong here. Why does it work?
    const float cosTheta = abs(dot(-wi, surface.m_Normal));

    if (importanceSampleBrdf)
        pdf = UE4JointPdf(specularWeight, nDotH, cosTheta, vDotH, surface.m_Roughness);
    else
        pdf = SampleDirectionHemisphere_Pdf();
}


GBufferSurface GetGBufferSurfaceFromTextures(float2 pixelCoord)
{
    const float4 gbuffer0 = g_GBufferOutput0.Load(int3(pixelCoord, 0));
    const float4 gbuffer1 = g_GBufferOutput1.Load(int3(pixelCoord, 0));
    const float4 gbuffer2 = g_GBufferOutput2.Load(int3(pixelCoord, 0));
    const float4 gbuffer3 = g_GBufferOutput3.Load(int3(pixelCoord, 0));

    GBufferSurface surface;
    surface.m_Position = gbuffer1.xyz;
    surface.m_Normal = DecodeNormals(gbuffer2.xy);
    surface.m_Albedo = gbuffer0.rgb;
    surface.m_Emission = gbuffer3.rgb * EMISSION_SCALE;
    surface.m_Roughness = gbuffer1.w;
    surface.m_Metalness = gbuffer0.w;
    surface.m_Velocity = gbuffer2.zw;

    return surface;
}

float EvaluateTargetFunctionForSurface(ReservoirSample rs, GBufferSurface surface)
{
    const float3 wi = normalize(rs.m_SamplePosition - surface.m_Position);
    const float3 wo = normalize(g_GlobalConstants.m_CameraPosition.xyz - surface.m_Position);
    const float3 f = BRDF_UE4(wi, wo, surface.m_Normal, surface.m_Albedo, surface.m_Roughness, surface.m_Metalness);
    const float cosTheta = max(0.1, saturate(dot(surface.m_Normal, wi)));
    return GetLuminanceFromRGB(rs.m_Radiance * f * cosTheta);
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
    float oldLengthSqr = ZERO_GUARD(dot(oldPath, oldPath));
    float newLengthSqr = ZERO_GUARD(dot(newPath, newPath));
    float oldCosAngle = ZERO_GUARD(saturate(dot(oldPath * rsqrt(oldLengthSqr), sharedNormal)));
    float newCosAngle = ZERO_GUARD(saturate(dot(newPath * rsqrt(newLengthSqr), sharedNormal)));

    float jacobian = (newCosAngle * oldLengthSqr) / (oldCosAngle * newLengthSqr);

	if (isinf(jacobian) || isnan(jacobian))
		jacobian = 0;

	return jacobian;
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

ReservoirSample TraceInitialSample(GBufferSurface surface, float3 wi)
{
    RayPayload payload;
    payload.m_IsShadowRay = false;
    payload.m_Depth = MAX_BOUNCES;
    payload.m_Hit = false;
    payload.m_Radiance = 0;
    payload.m_HitPosition = surface.m_Position + wi;
    payload.m_HitNormal = 0;

    RayDesc ray;
    ray.Direction = wi;
    ray.Origin = surface.m_Position;
    ray.TMax = 128;
    ray.TMin = 0.01;
    TraceRay(g_RaytracingTlas, RAY_FLAG_FORCE_OPAQUE, 0xFF, 0, 0, 0, ray, payload);

    ReservoirSample initialSample;
    initialSample.m_VisiblePosition = surface.m_Position;
    initialSample.m_VisibleNormal = surface.m_Normal;
    initialSample.m_SamplePosition = payload.m_HitPosition;
    initialSample.m_SampleNormal = payload.m_HitNormal;
    initialSample.m_Radiance = min(10000, payload.m_Radiance);

    return initialSample;
}

void GenerateInitialSamples()
{
    sampler linearSampler = SamplerDescriptorHeap[g_GlobalConstants.m_SamplerIndex_Linear_Clamp];
    const uint3 launchIndex = DispatchRaysIndex();
    const uint3 launchDim = DispatchRaysDimensions();
    const uint sampleIdx = launchIndex.y * launchDim.x + launchIndex.x;

    const GBufferSurface surface = GetGBufferSurfaceFromTextures(launchIndex.xy);
    const float3 viewDir = normalize(g_GlobalConstants.m_CameraPosition.xyz - surface.m_Position);
    const float viewDepth = length(surface.m_Position - g_GlobalConstants.m_CameraPosition.xyz);
    float3 wi;
    float proposalPdf;

#if USE_IMPORTANCE_SAMPLING
    SampleDirectionBrdf(surface, wi, proposalPdf);
#else
    SampleDirectionUniform(surface, wi, proposalPdf);
#endif

    ReservoirSample initialSample = TraceInitialSample(surface, wi);
    float targetPdf = EvaluateTargetFunctionForSurface(initialSample, surface);

    Reservoir Ri;
    Ri.m_Sample = initialSample;
    Ri.m_NumSamples = 1;
    Ri.m_TotalWeight = targetPdf / ZERO_GUARD(proposalPdf * Ri.m_NumSamples);
    Ri.m_UnbiasedContributionWeight = Ri.m_TotalWeight / ZERO_GUARD(Ri.m_NumSamples * targetPdf);

    g_ReSTIR_StagingReservoir[sampleIdx] = Ri;
}

void ApplyTemporalResampling()
{
    const uint3 launchIndex = DispatchRaysIndex();
    const uint3 launchDim = DispatchRaysDimensions();
    const uint sampleIdx = launchIndex.y * launchDim.x + launchIndex.x;

    const GBufferSurface surface = GetGBufferSurfaceFromTextures(launchIndex.xy);
    int2 launchIndexPrev = ((float2)launchIndex.xy + 0.5) - (surface.m_Velocity * launchDim.xy);

    //// Stochastic Bilinear Sampling
    //const float2 rand2D = CMJ_Sample2D(sampleIdx, 1024, 1024, g_GlobalConstants.m_FrameNumber);
    //launchIndexPrev.x = (rand2D.x >= frac(launchIndexPrev.x)) ? int(launchIndexPrev.x) : int(launchIndexPrev.x + 1);
    //launchIndexPrev.y = (rand2D.y >= frac(launchIndexPrev.y)) ? int(launchIndexPrev.y) : int(launchIndexPrev.y + 1);

    const uint sampleIdxPrev = clamp(launchIndexPrev.y * launchDim.x + launchIndexPrev.x, 0, launchDim.x * launchDim.y);

    Reservoir Ri = g_ReSTIR_StagingReservoir[sampleIdx];
    Reservoir Rt = g_ReSTIR_GIReservoir[sampleIdxPrev];

    if (!AreSamplesSimilar(Rt.m_Sample, Ri.m_Sample))
    {
        Rt.m_TotalWeight = 0;
    }

    if (launchIndexPrev.x < 0 || launchIndexPrev.x >= launchDim.x || 
        launchIndexPrev.y < 0 || launchIndexPrev.y >= launchDim.y)
        Rt.Reset();

#if DEBUG_RESET_HISTORY
    // Reset on move (DEBUG)
    if (g_GlobalConstants.m_FrameNumber == g_GlobalConstants.m_FrameSinceLastMovement)
        Rt.Reset();
#endif

    const float rand = Random(sampleIdx * g_GlobalConstants.m_FrameNumber);

#if USE_TEMPORAL_RESAMPLING

#if USE_JACOBIAN
    float jacobian = ComputeJacobianDeterminant(surface.m_Position, Rt.m_Sample.m_VisiblePosition, Rt.m_Sample.m_SamplePosition, Rt.m_Sample.m_SampleNormal);
    float targetFunction = EvaluateTargetFunctionForSurface(Rt.m_Sample, surface) * saturate(jacobian);
#else
    float targetFunction = EvaluateTargetFunctionForSurface(Rt.m_Sample, surface);
#endif

    Ri.Merge(Rt, targetFunction, rand, TEMPORAL_HISTORY);
    Ri.m_UnbiasedContributionWeight =  Ri.m_TotalWeight / ZERO_GUARD(Ri.m_NumSamples * EvaluateTargetFunctionForSurface(Ri.m_Sample, surface));
    //Ri.Combine(Rt, targetPdf, rand, TEMPORAL_HISTORY);
#endif

    g_ReSTIR_StagingReservoir[sampleIdx] = Ri;
}

void ApplySpatialResampling()
{
    const uint3 launchIndex = DispatchRaysIndex();
    const uint3 launchDim = DispatchRaysDimensions();
    const uint sampleIdx = launchIndex.y * launchDim.x + launchIndex.x;
    const GBufferSurface surface = GetGBufferSurfaceFromTextures(launchIndex.xy);

    Reservoir Rt = g_ReSTIR_StagingReservoir[sampleIdx];
    Reservoir Rs = Rt;

    const float viewDepth = length(surface.m_Position - g_GlobalConstants.m_CameraPosition.xyz);

    //const int numSpatialIterations = Rs.m_NumSamples < 30 ? 10 : 3;
    const int numSpatialIterations = 3;

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

        if (dot(RnNormal, surface.m_Normal) < 0.5)
            continue;
        if (abs(viewDepth - RnViewDepth) > 0.5)
            continue;

#if USE_JACOBIAN
        float jacobian = ComputeJacobianDeterminant(surface.m_Position, nSample.m_VisiblePosition, nSample.m_SamplePosition, nSample.m_SampleNormal);
        float targetFunction = EvaluateTargetFunctionForSurface(neighbour.m_Sample, surface) * saturate(jacobian);
#else
        float targetFunction = EvaluateTargetFunctionForSurface(neighbour.m_Sample, surface);
#endif

        //if (!TraceVisibility(nSample.m_SamplePosition, Rs.m_Sample.m_VisiblePosition))
        //    targetFunction = 0;

        const float rand = Random(sampleIdx * g_GlobalConstants.m_FrameNumber);
        Rs.Merge(neighbour, targetFunction, rand, SPATIAL_HISTORY);
    }

    Rs.m_UnbiasedContributionWeight = Rs.m_TotalWeight / ZERO_GUARD(Rs.m_NumSamples * EvaluateTargetFunctionForSurface(Rs.m_Sample, surface));


#if USE_SPATIAL_RESAMPLING
    Reservoir finalReservoir = Rs;
#else
    Reservoir finalReservoir = Rt;
#endif

    ReservoirSample finalSample = finalReservoir.m_Sample;

    float3 Lo_direct = 0;
    float3 Lo_indirect = 0;

    {   // Direct / shadow ray
        RayPayload payload;
        payload.m_IsShadowRay = true;

        RayDesc shadowRay;
        shadowRay.Direction = normalize(g_GlobalConstants.m_SunDirection).xyz;
        shadowRay.Origin = surface.m_Position + surface.m_Normal * 0.01;
        shadowRay.TMax = 128;
        shadowRay.TMin = 0.01;
        TraceRay(g_RaytracingTlas, RAY_FLAG_FORCE_OPAQUE | RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH, 0xFF, 0, 0, 0, shadowRay, payload);

        const float3 wi = normalize(shadowRay.Direction);
        const float3 wo = normalize(g_GlobalConstants.m_CameraPosition.xyz - surface.m_Position);
        const float3 Li = payload.m_Radiance;
        const float3 f = BRDF_UE4(wi, wo, surface.m_Normal, surface.m_Albedo, surface.m_Roughness, surface.m_Metalness);
        const float cosTheta = saturate(dot(wi, surface.m_Normal));
        Lo_direct = Li * f * cosTheta;
    }

    const float3 finalWi = normalize(finalSample.m_SamplePosition - surface.m_Position);
    const float3 finalWo = normalize(g_GlobalConstants.m_CameraPosition.xyz - surface.m_Position);
    const float cosTheta = saturate(dot(finalWi, surface.m_Normal));
    const float3 f = BRDF_UE4(finalWi, finalWo, surface.m_Normal, surface.m_Albedo, surface.m_Roughness, surface.m_Metalness);

    Lo_indirect = finalSample.m_Radiance * finalReservoir.m_UnbiasedContributionWeight * f * cosTheta;

#if DEBUG_GREYSCALE_OUTPUT
    Lo_indirect = GetLuminanceFromRGB(Lo_indirect);
#endif

    g_LightingOutput[launchIndex.xy].xyz = surface.m_Emission + Lo_direct + Lo_indirect;

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
