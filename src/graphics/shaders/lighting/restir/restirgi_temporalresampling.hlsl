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
#include "utils/shading.hlsl"
#include "common/globalconstants.h"
#include "common/raytracingconstants.h"
#include "common/material.h"
#include "lighting/brdf.hlsl"
#include "lighting/restir/gireservoirmanagement.hlsl"

ConstantBuffer<GlobalConstants> g_GlobalConstants           : register(b0);

RaytracingAccelerationStructure g_RaytracingTlas            : register(t0);
StructuredBuffer<GeometryInfo> g_GeometryInfo               : register(t1);
StructuredBuffer<Material> g_MaterialTable                  : register(t2);
Texture2D<float4> g_GBufferA                                : register(t3);
Texture2D<float4> g_GBufferB                                : register(t4);
Texture2D<float4> g_GBufferC                                : register(t5);
Texture2D<float4> g_GBufferD                                : register(t6);

RWStructuredBuffer<GIPackedReservoir> g_InputReservoir      : register(u0);
RWStructuredBuffer<GIPackedReservoir> g_RWOutputReservoir   : register(u1);

// TODO: Move to shading pass
RWTexture2D<float4> g_LightingOutput                        : register(u2);

#define INDIRECT_MIP_LEVEL 8
#define MAX_DEPTH 1

// TODO: implement various biases (pullback, surface, etc.)
#define RAY_TMAX 128
#define RAY_TMIN 0.1

float3 SampleEnvironmentLighting(float3 wi)
{
    sampler linearSampler = SamplerDescriptorHeap[g_GlobalConstants.m_SamplerIndex_Linear_Wrap];
    Texture2D<float4> hdriTexture = ResourceDescriptorHeap[g_GlobalConstants.m_HdriTextureIndex];
    const float exposure = 10000.0f;

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
    return surface.m_Emission + f * Li * cosTheta;
}

RayPayload TraceShadowRay(ShadingSurface surface)
{
    RayPayload payload;
    payload.m_IsShadowRay = true;
    payload.m_Depth = 0;

    RayDesc ray;
    ray.Direction = normalize(g_GlobalConstants.m_SunDirection).xyz;
    ray.Origin = surface.m_Position + surface.m_Normal * 0.01;
    ray.TMax = RAY_TMAX;
    ray.TMin = RAY_TMIN;
    TraceRay(g_RaytracingTlas, RAY_FLAG_FORCE_OPAQUE | RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH, 0xFF, 0, 0, 0, ray, payload);

    return payload;
}

RayPayload TraceShadingRay(float3 position, float3 direction, uint depth = 0)
{
    RayPayload payload;
    payload.m_IsShadowRay = false;
    payload.m_Depth = depth;

    RayDesc ray;
    ray.Direction = direction;
    ray.Origin = position;
    ray.TMax = RAY_TMAX;
    ray.TMin = RAY_TMIN;
    TraceRay(g_RaytracingTlas, RAY_FLAG_FORCE_OPAQUE, 0xFF, 0, 0, 0, ray, payload);

    return payload;
}

[shader("raygeneration")]
void RayGeneration()
{
    const uint2 screenCoords = DispatchRaysIndex().xy;
    const uint3 screenDims = DispatchRaysDimensions();
    const uint sampleIdx = screenCoords.y * screenDims.x + screenCoords.x;

    const ShadingSurface surface = GetShadingSurfaceFromGBuffers(screenCoords, g_GBufferA, g_GBufferB, g_GBufferC, g_GBufferD);

    const float3 wo = normalize(g_GlobalConstants.m_CameraPosition.xyz - surface.m_Position);
    const float3 wi = TangentToWorld(SampleDirectionHemisphere(CMJ_Sample2D(sampleIdx, 1024, 1024, g_GlobalConstants.m_FrameNumber + 100)), surface.m_Normal);
    const float pdf = SampleDirectionHemisphere_Pdf();

    const RayPayload payload = TraceShadingRay(surface.m_Position, wi, MAX_DEPTH);
    GIReservoirSample initialSample = GIReservoirSample::Empty();
    initialSample.m_Position = payload.m_HitPosition;
    initialSample.m_Normal = payload.m_HitNormal;
    initialSample.m_Radiance = ComputeRadiance(surface, payload.m_Radiance, wi, wo);

    const float targetFunction = log2(GetLuminanceFromRGB(initialSample.m_Radiance));
    const float risWeight = targetFunction / pdf;

    GIReservoir initialReservoir = GIReservoir::Empty();
    initialReservoir.Resample(initialSample, 1.0f, targetFunction, risWeight);
    g_RWOutputReservoir[sampleIdx] = GIReservoir::Pack(initialReservoir);

    // Temp output 
    const RayPayload shadowRay = TraceShadowRay(surface);
    const float3 directLighting = ComputeRadiance(surface, shadowRay.m_Radiance, normalize(g_GlobalConstants.m_SunDirection.xyz), wo);

    initialReservoir.FinalizeResampling();
    g_LightingOutput[screenCoords].xyz = directLighting + initialReservoir.m_Sample.m_Radiance * initialReservoir.m_WeightSum;
    g_LightingOutput[screenCoords].a = 0;
}

[shader("miss")]
void Miss(inout RayPayload payload)
{
    payload.m_Hit = false;
    payload.m_Radiance = 0;
    payload.m_HitPosition = WorldRayOrigin() + WorldRayDirection() * 9999.0f;

    if (payload.m_IsShadowRay)
    {
        // Sample sun color
        const float lerpFactor = saturate(dot(g_GlobalConstants.m_SunDirection.xyz, float3(0, 1, 0)));
        payload.m_Radiance = lerp(0.0f, g_GlobalConstants.m_SunColor.xyz, lerpFactor); // TODO: Make sun scale cvar or bake into suncolor
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
    const ShadingSurface surface = GetShadingSurfaceFromHit(vertex, material, g_GlobalConstants.m_SamplerIndex_Linear_Wrap, INDIRECT_MIP_LEVEL);

    payload.m_Hit = true;
    payload.m_HitPosition = surface.m_Position;
    payload.m_HitNormal = surface.m_Normal;

    if (payload.m_Depth <= 0)
    {
        payload.m_Radiance = surface.m_Emission;
    }
    else
    {
        const RayPayload shadowRay = TraceShadowRay(surface);
        const float3 wo = normalize(-WorldRayDirection());
        const float3 wi = normalize(g_GlobalConstants.m_SunDirection.xyz);
        payload.m_Radiance = ComputeRadiance(surface, shadowRay.m_Radiance, wi, wo);
    }
}
