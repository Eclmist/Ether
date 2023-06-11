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

RaytracingAccelerationStructure g_RaytracingTlas    : register(t0);
Texture2D<float4> g_AccumulationBuffer              : register(t1);
Texture2D<float4> g_GBufferOutput0                  : register(t2);
Texture2D<float4> g_GBufferOutput1                  : register(t3);
Texture2D<float4> g_GBufferOutput2                  : register(t4);

ConstantBuffer<GlobalConstants> g_GlobalConstants   : register(b0);
RWTexture2D<float4> g_Output                        : register(u0);

sampler g_PointSampler                              : register(s0);
sampler g_BilinearSampler                           : register(s1);

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
    uint3 launchIndex = DispatchRaysIndex();
    uint3 launchDim = DispatchRaysDimensions();
    float2 uv = (float2)launchIndex.xy / launchDim.xy + rcp((float2)launchDim.xy) / 2.0;

    float4 gbuffer1 = g_GBufferOutput1.Load(launchIndex);
    float4 gbuffer2 = g_GBufferOutput2.Load(launchIndex);

    float3 position = gbuffer1.xyz;
    float3 normal = float3(gbuffer1.w, gbuffer2.xy);
    float2 velocity = gbuffer2.zw;

    float a = g_GlobalConstants.m_TemporalAccumulationFactor;

    float3 sunDirection = normalize(g_GlobalConstants.m_SunDirection);
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
    ray.TMin = 0.001;
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
            light += (payload.m_RayT / 16) * skyColor * rcp(NumRays);
        else
            light += skyColor * rcp(NumRays);
    }

    float4 prevOutput = g_AccumulationBuffer.SampleLevel(g_BilinearSampler, uv - velocity, 0);
    prevOutput = max(ambientColor, prevOutput.xyz).xyzz;

    g_Output[launchIndex.xy] = (light * a) + (1 - a) * prevOutput;
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
