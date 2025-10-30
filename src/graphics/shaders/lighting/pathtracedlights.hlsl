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

#define USE_IMPORTANCE_SAMPLING 1

ConstantBuffer<GlobalConstants> g_GlobalConstants   : register(b0);
RaytracingAccelerationStructure g_RaytracingTlas    : register(t0);
StructuredBuffer<GeometryInfo> g_GeometryInfo       : register(t1);
StructuredBuffer<Material> g_MaterialTable          : register(t2);
Texture2D<float4> g_AccumulationTexture             : register(t3);
Texture2D<float4> g_GBufferA                        : register(t4);
Texture2D<float4> g_GBufferB                        : register(t5);
Texture2D<float4> g_GBufferC                        : register(t6);
Texture2D<float4> g_GBufferD                        : register(t7);
RWTexture2D<float4> g_LightingOutput                : register(u0);
RWTexture2D<float4> g_IndirectOutput                : register(u1);

float3 SampleEnvironmentLighting(float3 wi)
{
    sampler linearSampler = SamplerDescriptorHeap[g_GlobalConstants.m_SamplerIndex_Linear_Wrap];
    Texture2D<float4> hdriTexture = ResourceDescriptorHeap[g_GlobalConstants.m_HdriTextureIndex];
    const float exposure = 10000.0f;

    const float2 hdriUv = SampleSphericalMap(wi);
    const float4 hdri = hdriTexture.SampleLevel(linearSampler, hdriUv, 4);
    const float sunsetFactor = saturate(asin(dot(g_GlobalConstants.m_SunDirection.xyz, float3(0, 1, 0))));
    const float sunlightFactor = 1 - saturate(asin(dot(g_GlobalConstants.m_SunDirection.xyz, float3(0, -1, 0))));
    const float groundFactor = saturate(wi.y);

    const float4 color = lerp(float4(0.5, 0.25, 0.25, 0), 1, sunsetFactor) * sunlightFactor * groundFactor;

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

RayPayload TraceShadingRay(float3 position, float3 direction, uint depth)
{
    direction = normalize(direction);
    
    RayPayload payload;
    payload.m_IsShadowRay = false;
    payload.m_Depth = depth;
    payload.m_Radiance = 0.0f;

    if (depth <= 0)
        return payload;

    RayDesc ray;
    ray.Origin = position + direction * 0.01;
    ray.Direction = direction;
    ray.TMax = RAY_TMAX;
    ray.TMin = RAY_TMIN;
    TraceRay(g_RaytracingTlas, RAY_FLAG_FORCE_OPAQUE | RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH, 0xFF, 0, 0, 0, ray, payload);

    return payload;
}

void SampleDirectionBrdf(ShadingSurface surface, out float3 wi, out float pdf)
{
    const uint2 sampleCoords = DispatchRaysIndex().xy;
    const uint2 bufferSize = DispatchRaysDimensions().xy;
    const uint sampleIdx = sampleCoords.y * bufferSize.x + sampleCoords.x;
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
    const float cosTheta = abs(dot(wi, surface.m_Normal));

    if (importanceSampleBrdf)
        pdf = UE4JointPdf(specularWeight, nDotH, cosTheta, vDotH, surface.m_Roughness);
    else
        pdf = SampleDirectionHemisphere_Pdf();
}

void SampleDirectionUniform(ShadingSurface surface, out float3 wi, out float pdf)
{
    const uint2 sampleCoords = DispatchRaysIndex().xy;
    const uint2 bufferSize = DispatchRaysDimensions().xy;
    const uint sampleIdx = sampleCoords.y * bufferSize.x + sampleCoords.x;
    const float2 rand2D = CMJ_Sample2D(sampleIdx, 1024, 1024, g_GlobalConstants.m_FrameNumber);
    wi = TangentToWorld(SampleDirectionHemisphere(rand2D), surface.m_Normal);
    pdf = SampleDirectionHemisphere_Pdf();
}

[shader("raygeneration")]
void RayGeneration()
{
    const uint2 sampleCoords = DispatchRaysIndex().xy;
    const uint2 bufferSize = DispatchRaysDimensions().xy;
    const ShadingSurface surface = GetShadingSurfaceFromGBuffers(sampleCoords, g_GBufferA, g_GBufferB, g_GBufferC, g_GBufferD);

    const float3 viewDir = normalize(g_GlobalConstants.m_CameraPosition.xyz - surface.m_Position);
    const float2 uv = (float2) sampleCoords.xy / bufferSize.xy + rcp((float2) bufferSize.xy) / 2.0;
    const float2 uvPrev = uv - surface.m_Velocity;
    float4 accumulation = 0;

    sampler linearSampler = SamplerDescriptorHeap[g_GlobalConstants.m_SamplerIndex_Linear_Clamp];
    if (uvPrev.x >= 0 && uvPrev.x < 1 && uvPrev.y >= 0 && uvPrev.y < 1)
         accumulation = g_AccumulationTexture.SampleLevel(linearSampler, uvPrev, 0);

    const RayPayload shadowRay = TraceShadowRay(surface);
    const float3 direct = ComputeRadiance(surface, shadowRay.m_Radiance, g_GlobalConstants.m_SunDirection.xyz, viewDir);

    float3 wi;
    float pdf;

#if USE_IMPORTANCE_SAMPLING
    SampleDirectionBrdf(surface, wi, pdf);
#else
    SampleDirectionUniform(surface, wi, pdf);
#endif

    const RayPayload indirectRay = TraceShadingRay(surface.m_Position, wi, MAX_DEPTH);
    const float3 indirect = ComputeRadiance(surface, indirectRay.m_Radiance, wi, viewDir) / pdf;

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
    payload.m_HitPosition = WorldRayOrigin() + WorldRayDirection() * 9999999.0f;

    if (payload.m_IsShadowRay)
    {
        // Sample sun color
        const float lerpFactor = saturate(dot(g_GlobalConstants.m_SunDirection.xyz, float3(0, 1, 0)));
        payload.m_Radiance = lerp(0.0f, g_GlobalConstants.m_SunColor.xyz, lerpFactor);
    }
    else
    {
        // Sample environment color
        payload.m_Radiance = SampleEnvironmentLighting(WorldRayDirection());
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

    float3 direct;
    float3 indirect;

    {   // Direct lighting
        const RayPayload shadowRay = TraceShadowRay(surface);
        direct = ComputeRadiance(surface, shadowRay.m_Radiance, g_GlobalConstants.m_SunDirection.xyz, -WorldRayDirection());
    }

    {   // Indirect lighting
        float3 wi;
        float pdf;

#if USE_IMPORTANCE_SAMPLING
        SampleDirectionBrdf(surface, wi, pdf);
#else
        SampleDirectionUniform(surface, wi, pdf);
#endif
        
        const RayPayload indirectRay = TraceShadingRay(surface.m_Position, wi, payload.m_Depth);
        indirect = ComputeRadiance(surface, indirectRay.m_Radiance, wi, -WorldRayDirection()) / pdf;
    }

    payload.m_Radiance = surface.m_Emission + direct + indirect;
}
