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
#include "common/raypayload.h"

ConstantBuffer<GlobalConstants> g_GlobalConstants   : register(b0);
RaytracingAccelerationStructure g_RaytracingTlas    : register(t0);
Texture2D<float4> g_GBufferOutput0                  : register(t1);
Texture2D<float4> g_GBufferOutput1                  : register(t2);
Texture2D<float4> g_GBufferOutput2                  : register(t3);
Texture2D<float4> g_GBufferOutput3                  : register(t4);
RWTexture2D<float4> g_LightingOutput                : register(u0);

// 3D Gradient noise from: https://www.shadertoy.com/view/Xsl3Dl
float3 hash(float3 p)
{
    p = float3(
        dot(p, float3(127.1, 311.7, 74.7)),
        dot(p, float3(269.5, 183.3, 246.1)),
        dot(p, float3(113.5, 271.9, 124.6)));

    return -1.0 + 2.0 * frac(sin(p) * 43758.5453123);
}

float3 uniformRandomDirection(float3 s)
{
    float3 r = hash(s);
    return normalize(r / cos(r));
}

[shader("raygeneration")]
void RayGeneration()
{
    sampler linearSampler = SamplerDescriptorHeap[g_GlobalConstants.m_SamplerIndex_Linear_Clamp];

    uint3 launchIndex = DispatchRaysIndex();
    uint3 launchDim = DispatchRaysDimensions();
    float2 uv = (float2)launchIndex.xy / launchDim.xy + rcp((float2)launchDim.xy) / 2.0;

    float4 gbuffer1 = g_GBufferOutput1.Load(launchIndex);
    float4 gbuffer2 = g_GBufferOutput2.Load(launchIndex);

    float3 position = gbuffer1.xyz;
    float3 normal = gbuffer2.xyz;
    float roughness = gbuffer1.w;
    float metalness = gbuffer2.w;

    float3 sunDirection = normalize(g_GlobalConstants.m_SunDirection).xyz;
    float4 sunColor = g_GlobalConstants.m_SunColor;
    float4 nightSkyColor = float4(0.1, 0.25, 0.4, 1.0);
    float4 daySkyColor = sunColor;
    float4 skyColor = lerp(nightSkyColor, daySkyColor, dot(sunDirection, float3(0, 1, 0)));

    float4 ambientColor = skyColor * 0.05;
    float4 light = ambientColor;

    RayPayload payload;
    RayDesc ray;

    ray.Direction = sunDirection;
    ray.Origin = position;
    ray.TMax = 64;
    ray.TMin = 0.01;
    TraceRay(g_RaytracingTlas, 0, 0xFF, 0, 0, 0, ray, payload);

    if (!payload.m_Hit)
        light += sunColor * saturate(dot(normal, sunDirection)) * 2;

    const int NumRays = 1;
    for (int i = 0; i < NumRays; ++i)
    {
        ray.Origin = position;
        ray.Direction = normalize(normal.xyz + roughness * normalize(uniformRandomDirection(position + (g_GlobalConstants.m_Time.w % 0.31415923))));
        TraceRay(g_RaytracingTlas, 0, 0xFF, 0, 0, 0, ray, payload);

        if (payload.m_Hit)
            light += pow(saturate(payload.m_RayT / 16), 1.5) * skyColor * rcp(NumRays);
        else
            light += skyColor * rcp(NumRays);
    }

    g_LightingOutput[launchIndex.xy].xyz = light.xyz;
}

[shader("miss")]
void Miss(inout RayPayload payload)
{
    payload.m_Hit = false;
}

[shader("closesthit")]
void ClosestHit(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attribs)
{
    payload.m_Hit = true;
    payload.m_RayT = RayTCurrent();
}
