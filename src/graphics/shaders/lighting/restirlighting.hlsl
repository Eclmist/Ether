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
#define USE_JACOBIAN                0

#define MAX_TEMPORAL_HISTORY        30
#define MAX_SPATIAL_HISTORY         500
#define SPATIAL_RADIUS              50

#define DEBUG_GREYSCALE_OUTPUT      0
#define DEBUG_RESET_HISTORY         0
#define DEBUG_SPLIT_SCREEN_PASSES   0

// TODO: Make lightingcommon.h
#define EMISSION_SCALE              10000
#define SUNLIGHT_SCALE              120000
#define SKYLIGHT_SCALE              10000
#define POINTLIGHT_SCALE            2000
#define MAX_BOUNCES                 0

#define ZERO_GUARD(f)               (max(0.001, f))

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

ConstantBuffer<GlobalConstants> g_GlobalConstants : register(b0);
ConstantBuffer<RootConstants> g_RootConstants : register(b1);

RaytracingAccelerationStructure g_RaytracingTlas : register(t0);
StructuredBuffer<GeometryInfo> g_GeometryInfo : register(t1);
StructuredBuffer<Material> g_MaterialTable : register(t2);
Texture2D<float4> g_GBufferOutput0 : register(t3);
Texture2D<float4> g_GBufferOutput1 : register(t4);
Texture2D<float4> g_GBufferOutput2 : register(t5);
Texture2D<float4> g_GBufferOutput3 : register(t6);
RWTexture2D<float4> g_LightingOutput : register(u0);

/* ReSTIR GI Implementation */
RWStructuredBuffer<Reservoir> g_ReSTIR_GIReservoir : register(u1);
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

float3 EvaluateBxDF(float3 wi, float3 wo, GBufferSurface surface)
{
    return BRDF_UE4(wi, wo, surface.m_Normal, surface.m_Albedo, surface.m_Roughness, surface.m_Metalness);
}

float EvaluateTargetFunction(ReservoirSample rs, GBufferSurface surface)
{
    const float3 wi = normalize(rs.m_SamplePosition - surface.m_Position);
    const float3 wo = normalize(g_GlobalConstants.m_CameraPosition.xyz - surface.m_Position);
    const float3 f = EvaluateBxDF(wi, wo, surface);
    const float cosTheta = max(0.01, saturate(dot(surface.m_Normal, wi)));
    return GetLuminanceFromRGB(rs.m_Radiance * cosTheta * f);
}

float3 EvaluateFinalIndirect(Reservoir finalReservoir, GBufferSurface surface)
{
    finalReservoir.FinalizeReservoir();
    ReservoirSample finalSample = finalReservoir.m_Sample;

    const float3 wi = normalize(finalSample.m_SamplePosition - surface.m_Position);
    const float3 wo = normalize(g_GlobalConstants.m_CameraPosition.xyz - surface.m_Position);
    const float cosTheta = saturate(dot(wi, surface.m_Normal));
    const float3 f = EvaluateBxDF(wi, wo, surface);

    const float ucw = finalReservoir.m_WeightSum; // weight sum becomes unbiased contribution weight after FializeReservoir();
    return finalSample.m_Radiance * ucw * f * cosTheta;
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
    const uint3 launchIndex = DispatchRaysIndex();
    const uint3 launchDim = DispatchRaysDimensions();
    const uint sampleIdx = launchIndex.y * launchDim.x + launchIndex.x;
    const float2 rand2D = CMJ_Sample2D(sampleIdx, 1024, 1024, g_GlobalConstants.m_FrameNumber);

    const float3 wo = normalize(g_GlobalConstants.m_CameraPosition.xyz - surface.m_Position);
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

    // Flip normal if backface. This fixes a lot of light leakage
    if (dot(surface.m_Normal, normalize(g_GlobalConstants.m_CameraPosition - surface.m_Position)) < 0)
        surface.m_Normal = -surface.m_Normal;

    return surface;
}

bool AreDepthSimilar(float3 a, float3 b)
{
    const float aDepth = length(a - g_GlobalConstants.m_CameraPosition.xyz);
    const float bDepth = length(b - g_GlobalConstants.m_CameraPosition.xyz);
    return abs(aDepth - bDepth) < 10;
}

bool AreSurfacesSimilar(GBufferSurface a, GBufferSurface b)
{
    if (dot(a.m_Normal, b.m_Normal) < 0.9)
        return false;

    if (!AreDepthSimilar(a.m_Position, b.m_Position))
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

float3 TraceDirectLighting(GBufferSurface surface, float3 wo)
{
    RayPayload payload;
    payload.m_IsShadowRay = true;

    // Flip normal if backface. This fixes a lot of light leakage
    surface.m_Normal = dot(wo, surface.m_Normal) < 0 ? -surface.m_Normal : surface.m_Normal; 

    RayDesc shadowRay;
    shadowRay.Direction = normalize(g_GlobalConstants.m_SunDirection).xyz;
    shadowRay.Origin = surface.m_Position + surface.m_Normal * 0.01;
    shadowRay.TMax = 128;
    shadowRay.TMin = 0.01;
    uint flags = RAY_FLAG_FORCE_OPAQUE | RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH;
    TraceRay(g_RaytracingTlas, flags, 0xFF, 0, 0, 0, shadowRay, payload);

    const float3 wi = normalize(shadowRay.Direction);
    const float3 Li = payload.m_Radiance;
    const float3 f = BRDF_UE4(wi, wo, surface.m_Normal, surface.m_Albedo, surface.m_Roughness, surface.m_Metalness);
    const float cosTheta = saturate(dot(wi, surface.m_Normal));
    return Li * f * cosTheta;
}

float3 TraceLightingRecursively(GBufferSurface surface, uint depth)
{
    const uint3 launchIndex = DispatchRaysIndex();
    const uint3 launchDim = DispatchRaysDimensions();
    const uint sampleIdx = launchIndex.y * launchDim.x + launchIndex.x;
    const float2 rand2D = CMJ_Sample2D(sampleIdx, 1024, 1024, g_GlobalConstants.m_FrameNumber);
    const float3 wo = normalize(-WorldRayDirection());
    surface.m_Normal = dot(wo, surface.m_Normal) < 0 ? -surface.m_Normal : surface.m_Normal; 

// 0 -> Importance sample BRDF
// 1 -> Importance sample Cosine Hemisphere
// 2 -> Importance sample Hemisphere
#define IMPORTANCE_SAMPLING 2

#if IMPORTANCE_SAMPLING == 0
    const float diffuseWeight = lerp(lerp(0.5, 1.0, surface.m_Roughness), 0.0, surface.m_Metalness);
    const float specularWeight = 1.0 - diffuseWeight;
    float3 wi = 0;
    if (Random(rand2D.x) <= specularWeight)
        wi = ImportanceSampleGGX(rand2D, wo, surface.m_Normal, surface.m_Roughness);
    else
        wi = TangentToWorld(SampleDirectionCosineHemisphere(rand2D), surface.m_Normal);

    const float3 H = normalize(wi + wo);
    const float nDotH = saturate(dot(surface.m_Normal, H));
    const float nDotV = saturate(dot(surface.m_Normal, wo));
    const float vDotH = saturate(dot(wo, H));
    const float cosTheta = saturate(dot(wi, surface.m_Normal));
    const float pdf = UE4JointPdf(specularWeight, nDotH, cosTheta, vDotH, surface.m_Roughness);
#elif IMPORTANCE_SAMPLING == 1
    float3 wi = TangentToWorld(SampleDirectionCosineHemisphere(rand2D), surface.m_Normal);
    const float cosTheta = saturate(dot(wi, surface.m_Normal));
    const float pdf = cosTheta / Pi;
#else
    float3 wi = TangentToWorld(SampleDirectionHemisphere(rand2D), surface.m_Normal);
    const float cosTheta = saturate(dot(wi, surface.m_Normal));
    const float pdf = 1 / Pi2;
#endif

    float3 f = 0;
    if (cosTheta > 0 && pdf > 0)
        f = BRDF_UE4(wi, wo, surface.m_Normal, surface.m_Albedo, surface.m_Roughness, surface.m_Metalness) / pdf;

    float3 Li = 0;
    if (depth >= MAX_BOUNCES)
    {
        return 0;
    }
    else
    {
        RayPayload indirectPayload;
        indirectPayload.m_IsShadowRay = false;
        indirectPayload.m_Depth = depth + 1;

        RayDesc indirectRay;
        indirectRay.Direction = wi;
        indirectRay.Origin = surface.m_Position;
        indirectRay.TMax = 128;
        indirectRay.TMin = 0.01;
        TraceRay(g_RaytracingTlas, RAY_FLAG_FORCE_OPAQUE, 0xFF, 0, 0, 0, indirectRay, indirectPayload);
        Li = indirectPayload.m_Radiance;
    }

    return Li * f * cosTheta;
}

ReservoirSample TraceInitialSample(GBufferSurface surface, float3 wi, float3 wo)
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

    ReservoirSample initialSample = TraceInitialSample(surface, wi, viewDir);
    float targetFunction = EvaluateTargetFunction(initialSample, surface);

    Reservoir Ri;
    Ri.m_Sample = initialSample;
    Ri.m_NumSamples = 1;
    Ri.m_TargetFunction = targetFunction;
    Ri.m_WeightSum = proposalPdf > 0.0f ? (targetFunction / proposalPdf * Ri.m_NumSamples) : 0.0f; // GRIS formulation, 1/M * (p^ / p)
    Ri.FinalizeReservoir();
    g_ReSTIR_StagingReservoir[sampleIdx] = Ri;
}

void ApplyTemporalResampling()
{
    const uint3 launchIndex = DispatchRaysIndex();
    const uint3 launchDim = DispatchRaysDimensions();
    const uint sampleIdx = launchIndex.y * launchDim.x + launchIndex.x;
    const GBufferSurface surface = GetGBufferSurfaceFromTextures(launchIndex.xy);

    //// Stochastic Bilinear Sampling
    // const float2 rand2D = CMJ_Sample2D(sampleIdx, 1024, 1024, g_GlobalConstants.m_FrameNumber);
    // launchIndexPrev.x = (rand2D.x >= frac(launchIndexPrev.x)) ? int(launchIndexPrev.x) : int(launchIndexPrev.x + 1);
    // launchIndexPrev.y = (rand2D.y >= frac(launchIndexPrev.y)) ? int(launchIndexPrev.y) : int(launchIndexPrev.y + 1);


    Reservoir initialReservoir = g_ReSTIR_StagingReservoir[sampleIdx];
    Reservoir finalReservoir;
    finalReservoir.Reset();
    finalReservoir.Combine(initialReservoir, initialReservoir.m_TargetFunction, 0.0f);

#if USE_TEMPORAL_RESAMPLING

#if DEBUG_RESET_HISTORY
    uint numReprojections = g_GlobalConstants.m_FrameNumber == g_GlobalConstants.m_FrameSinceLastMovement 
        ? 0
#if DEBUG_SPLIT_SCREEN_PASSES
        : launchIndex.x < (launchDim.x / 3) ? 0 : 1;
#else
        : 1;
#endif

#else
    const uint numReprojections = 1;
#endif

    for (uint i = 0; i < numReprojections; ++i)
    {
        int2 launchIndexPrev = ((float2)launchIndex.xy + 0.5) - (surface.m_Velocity * launchDim.xy);
        const float2 rand2D = CMJ_Sample2D(sampleIdx, 1024, 1024, g_GlobalConstants.m_FrameNumber * i);
        const int2 offset = (SquareToConcentricDiskMapping(rand2D) - 0.5f) * 4 * (i / (float)numReprojections);

        launchIndexPrev += offset;
        launchIndexPrev = clamp(launchIndexPrev, 0, launchDim);
        const uint sampleIdxPrev = clamp(launchIndexPrev.y * launchDim.x + launchIndexPrev.x, 0, launchDim.x * launchDim.y);
    
        Reservoir reprojectedReservoir = g_ReSTIR_GIReservoir[sampleIdxPrev];
        ReservoirSample reprojectedSample = reprojectedReservoir.m_Sample;
        GBufferSurface reprojectedSurface = GetGBufferSurfaceFromTextures(launchIndexPrev.xy);

        if (!AreSurfacesSimilar(surface, reprojectedSurface))
            reprojectedReservoir.m_NumSamples = 0;


#if USE_JACOBIAN
        float jacobian = ComputeJacobianDeterminant(surface.m_Position, reprojectedSurface.m_Position, reprojectedSample.m_SamplePosition, reprojectedSample.m_SampleNormal);
        reprojectedReservoir.m_WeightSum *= jacobian;

#endif
        const float targetFunction = EvaluateTargetFunction(reprojectedReservoir.m_Sample, surface);
        const float random = Random(sampleIdx * g_GlobalConstants.m_FrameNumber);
        reprojectedReservoir.m_NumSamples = min(reprojectedReservoir.m_NumSamples, MAX_TEMPORAL_HISTORY);
        finalReservoir.Combine(reprojectedReservoir, targetFunction, random);
        break;
    }
#endif // USE_TEMPORAL_RESAMPLING

    finalReservoir.FinalizeReservoir();
    g_ReSTIR_StagingReservoir[sampleIdx] = finalReservoir;
}

void ApplySpatialResampling()
{
    const uint3 launchIndex = DispatchRaysIndex();
    const uint3 launchDim = DispatchRaysDimensions();
    const uint sampleIdx = launchIndex.y * launchDim.x + launchIndex.x;
    const GBufferSurface surface = GetGBufferSurfaceFromTextures(launchIndex.xy);

    Reservoir temporalReservoir = g_ReSTIR_StagingReservoir[sampleIdx];
    Reservoir finalReservoir;
    finalReservoir.Reset();
    finalReservoir.Combine(temporalReservoir, temporalReservoir.m_TargetFunction, 0.0f);

#if USE_SPATIAL_RESAMPLING
    const float viewDepth = length(surface.m_Position - g_GlobalConstants.m_CameraPosition.xyz);
    // const int numSpatialIterations = Rs.m_NumSamples < 30 ? 10 : 3;

    const uint numSpatialIterations =
#if DEBUG_SPLIT_SCREEN_PASSES
        launchIndex.x > (launchDim.x * 2 / 3) ? 3 : 0;
#else
        3;
#endif

    for (uint i = 0; i < numSpatialIterations; ++i)
    {
        // Review: do random context with seed
        const float2 rand2D = CMJ_Sample2D(sampleIdx, 1024, 1024, g_GlobalConstants.m_FrameNumber * i);
        const int2 neighbourCoords = clamp((launchIndex.xy + 0.5f) + SquareToConcentricDiskMapping(rand2D) * (SPATIAL_RADIUS / numSpatialIterations * i), 0, launchDim);
        const uint neighbourIdx = clamp(neighbourCoords.y * launchDim.x + neighbourCoords.x, 0, launchDim.x * launchDim.y);
        Reservoir neighbour = g_ReSTIR_StagingReservoir[neighbourIdx];
        ReservoirSample nSample = neighbour.m_Sample;
        const GBufferSurface nSurface = GetGBufferSurfaceFromTextures(neighbourCoords);

        const float3 RnPosition = nSurface.m_Position;
        const float3 RnNormal = nSurface.m_Normal;
        const float RnViewDepth = length(RnPosition - g_GlobalConstants.m_CameraPosition.xyz);

        if (dot(RnNormal, surface.m_Normal) < 0.5)
            continue;
        if (abs(viewDepth - RnViewDepth) > 0.5)
            continue;

#if USE_JACOBIAN
        float jacobian = ComputeJacobianDeterminant(surface.m_Position, nSurface.m_Position, nSample.m_SamplePosition, nSample.m_SampleNormal);
        neighbour.m_WeightSum *= jacobian;
#endif // USE_JACOBIAN

        float targetFunction = EvaluateTargetFunction(neighbour.m_Sample, surface);

        const float rand = Random(sampleIdx * g_GlobalConstants.m_FrameNumber);
        neighbour.m_NumSamples = min(neighbour.m_NumSamples, MAX_SPATIAL_HISTORY);
        finalReservoir.Combine(neighbour, targetFunction, rand);
    }
#endif // USE_SPATIAL_RESAMPLING

    float3 Lo_indirect = EvaluateFinalIndirect(finalReservoir, surface);
    float3 Lo_direct = TraceDirectLighting(surface, normalize(g_GlobalConstants.m_CameraPosition.xyz - surface.m_Position));

#if DEBUG_GREYSCALE_OUTPUT
    Lo_indirect = GetLuminanceFromRGB(Lo_indirect);
#endif

    g_LightingOutput[launchIndex.xy].xyz = surface.m_Emission + Lo_direct + Lo_indirect;

#if DEBUG_SPLIT_SCREEN_PASSES
    if (launchIndex.x == launchDim.x / 3)
        g_LightingOutput[launchIndex.xy].xyz = float3(1, 0, 0) * 100000;

    if (launchIndex.x == launchDim.x * 2 / 3)
        g_LightingOutput[launchIndex.xy].xyz = float3(1, 0, 0) * 100000;
#endif

#if USE_SPATIAL_ACCUMULATION && USE_SPATIAL_RESAMPLING
    g_ReSTIR_GIReservoir[sampleIdx] = finalReservoir;
#else
    g_ReSTIR_GIReservoir[sampleIdx] = temporalReservoir;
#endif
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

    payload.m_HitPosition = hitSurface.m_Position;
    payload.m_HitNormal = normal;

    GBufferSurface virtualSurface;
    virtualSurface.m_Position = hitSurface.m_Position;
    virtualSurface.m_Normal = normal;
    virtualSurface.m_Albedo = albedo;
    virtualSurface.m_Roughness = roughness;
    virtualSurface.m_Metalness = metalness;
    virtualSurface.m_Emission = emission;

    const float3 Lo_direct = TraceDirectLighting(virtualSurface, -WorldRayDirection());
    const float3 Lo_indirect = TraceLightingRecursively(virtualSurface, payload.m_Depth);

    payload.m_Radiance = emission.xyz + Lo_direct + Lo_indirect;
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
        const float lerpFactor = saturate(dot(g_GlobalConstants.m_SunDirection.xyz, float3(0, 1, 0)));
        payload.m_Radiance = lerp(0.0f, g_GlobalConstants.m_SunColor.xyz * SUNLIGHT_SCALE, lerpFactor);
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

    if (!payload.m_IsShadowRay)
    {
        const GeometryInfo geoInfo = g_GeometryInfo[InstanceIndex()];
        const MeshVertex vertex = GetHitSurface(attribs, geoInfo);
        const Material material = g_MaterialTable[geoInfo.m_MaterialIndex];

        PathTrace(vertex, material, payload);
    }
}
