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

#define THREADGROUP_SIZE 8

ConstantBuffer<GlobalConstants> g_GlobalConstants           : register(b0);

RaytracingAccelerationStructure g_RaytracingTlas            : register(t0);
StructuredBuffer<GeometryInfo> g_GeometryInfo               : register(t1);
StructuredBuffer<Material> g_MaterialTable                  : register(t2);

Texture2D<float4> g_GBufferA                                : register(t3);
Texture2D<float4> g_GBufferB                                : register(t4);
Texture2D<float4> g_GBufferC                                : register(t5);
Texture2D<float4> g_GBufferD                                : register(t6);

RWStructuredBuffer<GIPackedReservoir> g_InputReservoir      : register(u0);
RWStructuredBuffer<GIPackedReservoir> g_HistoryReservoir    : register(u1);
RWStructuredBuffer<GIPackedReservoir> g_RWOutputReservoir   : register(u2);

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

float3 ComputeRadiance(ShadingSurface surface, GIReservoirSample sample)
{
    const float3 wi = normalize(sample.m_Position - surface.m_Position);
    const float3 wo = normalize(g_GlobalConstants.m_CameraPosition.xyz - surface.m_Position);
    return ComputeRadiance(surface, sample.m_Radiance, wi, wo);
}

float EvaluateTargetFunction(ShadingSurface surface, GIReservoirSample sample)
{
    const float3 L = ComputeRadiance(surface, sample);
    return GetLuminanceFromRGB(L);
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
    ray.Origin = position + direction * 0.01;
    ray.TMax = RAY_TMAX;
    ray.TMin = RAY_TMIN;
    TraceRay(g_RaytracingTlas, RAY_FLAG_FORCE_OPAQUE, 0xFF, 0, 0, 0, ray, payload);

    return payload;
}

RayPayload TraceValidationRay(ShadingSurface surface, GIReservoirSample sample)
{
    RayPayload payload;
    payload.m_IsShadowRay = true;
    payload.m_Depth = 0;

    RayDesc ray;
    ray.Direction = normalize(sample.m_Position - surface.m_Position);
    ray.Origin = surface.m_Position + surface.m_Normal * 0.01;
    ray.TMax = length(sample.m_Position - surface.m_Position) * 0.95f;
    ray.TMin = RAY_TMIN;
    TraceRay(g_RaytracingTlas, RAY_FLAG_FORCE_OPAQUE | RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH, 0xFF, 0, 0, 0, ray, payload);

    return payload;
}


