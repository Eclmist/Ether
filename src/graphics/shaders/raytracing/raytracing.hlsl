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
Texture2D<float4> g_AccumulationTex                 : register(t1);
Texture2D<float4> g_GBufferOutput0                  : register(t2);
Texture2D<float4> g_GBufferOutput1                  : register(t3);
Texture2D<float4> g_GBufferOutput2                  : register(t4);

ConstantBuffer<GlobalConstants> g_GlobalConstants   : register(b0);
RWTexture2D<float4> g_Output                        : register(u0);

sampler g_PointSampler                              : register(s0);
sampler g_BilinearSampler                           : register(s1);

float3 linearToSrgb(float3 c)
{
    // Based on http://chilliant.blogspot.com/2012/08/srgb-approximations-for-hlsl.html
    float3 sq1 = sqrt(c);
    float3 sq2 = sqrt(sq1);
    float3 sq3 = sqrt(sq2);
    float3 srgb = 0.662002687 * sq1 + 0.684122060 * sq2 - 0.323583601 * sq3 - 0.0225411470 * c;
    return srgb;
}

float hash(float2 p)
{
    return frac(dot(p + float2(.36834, .723), normalize(frac(p.yx * 73.91374) + 1e-4)) * 7.38734);
}

float3 nonUniformRandomDirection(float2 s)
{
    float3 r = float3(hash(s), hash(s * .8723), hash(s * .9574)) * 2. - 1.;
    return normalize(r);
}

float3 uniformRandomDirection(float2 s)
{
    float3 r = float3(hash(s), hash(s * .8723), hash(s * .9574)) * 2. - 1.;
    return normalize(r / cos(r));
}

[shader("raygeneration")]
void RayGeneration()
{
    uint3 launchIndex = DispatchRaysIndex();
    uint3 launchDim = DispatchRaysDimensions();

    float2 texCoord = (float2)launchIndex.xy / launchDim.xy + (rcp(launchDim.xy) / 2);

    float4 gbuffer0 = g_GBufferOutput0.SampleLevel(g_PointSampler, texCoord, 0);
    float4 gbuffer1 = g_GBufferOutput1.SampleLevel(g_PointSampler, texCoord, 0);
    float4 gbuffer2 = g_GBufferOutput2.SampleLevel(g_PointSampler, texCoord, 0);

    float4 color = gbuffer0;
    float3 position = gbuffer1.xyz;
    float3 normal = float3(gbuffer1.w, gbuffer2.xy);
    float2 velocity = gbuffer2.zw;

    float a = g_GlobalConstants.m_TemporalAccumulationFactor;
    float4 light = 1.0;

    float4 skyColor = float4(0.5, 0.7, 0.9, 1);
    float4 sunColor = float4(1, 0.85, 0.8, 1);
    float3 sunDirction = normalize(float3(0.4, 1, 0.10));

    if (color.w < 1)
    {
        g_Output[launchIndex.xy] = skyColor;
        return;
    }

    RayPayload payload;
    RayDesc ray;

    ray.Direction = sunDirction;
    ray.Origin = position;
    ray.TMax = 999;
    ray.TMin = 0;
    TraceRay(g_RaytracingTlas, RAY_FLAG_CULL_BACK_FACING_TRIANGLES, 0xFF, 0, 0, 0, ray, payload);

    if (!payload.m_Hit)
        light += sunColor * saturate(dot(normal, sunDirction)) * 7;

    ray.Origin = position;
    ray.Direction = normalize(normal.xyz + normalize(nonUniformRandomDirection((texCoord * 2 - 1) + (g_GlobalConstants.m_FrameNumber % 4096.) * 0.123)));
    ray.TMin = 0;
    ray.TMax = 32;
    TraceRay(g_RaytracingTlas, RAY_FLAG_CULL_BACK_FACING_TRIANGLES, 0xFF, 0, 0, 0, ray, payload);

    if (payload.m_Hit && color.w == 1)
        light *= saturate(0.15 + saturate(payload.m_RayT / ray.TMax));
    else
        light += skyColor * saturate(dot(normal, ray.Direction));

    float2 texCoordPrev = texCoord - velocity;
    float4 prevOutput = g_AccumulationTex.SampleLevel(g_PointSampler, velocity, 0);

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
