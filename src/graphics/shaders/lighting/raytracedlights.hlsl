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
RWTexture2D<float4> g_LightingOutput                : register(u0);

float hash(float2 p)
{
    return frac(dot(p + float2(.368434, .7263), normalize(frac(p.yx * 723.91374) + 1e-4)) * 7.382734);
}

float3 nonUniformRandomDirection(float2 s)
{
    float3 r = float3(hash(s), hash(s * .87243), hash(s * .95714)) * 2. - 1.;
    return normalize(r);
}

float3 uniformRandomDirection(float2 s)
{
    float3 r = float3(hash(s), hash(s * .872233), hash(s * .95574)) * 2. - 1.;
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
    float3 normal = float3(gbuffer1.w, gbuffer2.xy);

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
    TraceRay(g_RaytracingTlas, RAY_FLAG_CULL_BACK_FACING_TRIANGLES, 0xFF, 0, 0, 0, ray, payload);

    if (!payload.m_Hit)
        light += sunColor * saturate(dot(normal, sunDirection)) * 2;

    const int NumRays = 1;
    for (int i = 0; i < NumRays; ++i)
    {
        ray.Origin = position;
        ray.Direction = normalize(normal.xyz + normalize(nonUniformRandomDirection((float2(launchIndex.xy) + (g_GlobalConstants.m_FrameNumber * (i + 1) % 4096.) * 0.123))));
        TraceRay(g_RaytracingTlas, RAY_FLAG_CULL_BACK_FACING_TRIANGLES, 0xFF, 0, 0, 0, ray, payload);

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
