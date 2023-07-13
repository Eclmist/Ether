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
#include "utils/sampling.hlsl"

ConstantBuffer<GlobalConstants> g_GlobalConstants   : register(b0);
RaytracingAccelerationStructure g_RaytracingTlas    : register(t0);
Texture2D<float4> g_GBufferOutput0                  : register(t1);
Texture2D<float4> g_GBufferOutput1                  : register(t2);
Texture2D<float4> g_GBufferOutput2                  : register(t3);
Texture2D<float4> g_GBufferOutput3                  : register(t4);
RWTexture2D<float4> g_LightingOutput                : register(u0);

[shader("raygeneration")]
void RayGeneration()
{
    sampler linearSampler = SamplerDescriptorHeap[g_GlobalConstants.m_SamplerIndex_Linear_Clamp];

    uint3 launchIndex = DispatchRaysIndex();
    uint3 launchDim = DispatchRaysDimensions();
    float2 uv = (float2)launchIndex.xy / launchDim.xy + rcp((float2)launchDim.xy) / 2.0;
    uint sampleIdx = launchIndex.y * launchDim.x + launchIndex.x;

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

    float4 ambientColor = skyColor * 0.6;
    float4 light = ambientColor;
    float aoIntensity = g_GlobalConstants.m_RaytracedAOIntensity;

    RayPayload payload;
    RayDesc shadowRay;

    shadowRay.Direction = sunDirection;
    shadowRay.Origin = position;
    shadowRay.TMax = 32;
    shadowRay.TMin = 0.01;
    TraceRay(g_RaytracingTlas, 0, 0xFF, 0, 0, 0, shadowRay, payload);

    if (!payload.m_Hit)
        light += sunColor * saturate(dot(normal, sunDirection)) * 2.5;

    const int NumRays = 1;
    for (int i = 0; i < NumRays; ++i)
    {
        RayDesc aoRay;
        float3 ranSphere = SampleDirectionSphere(
            CMJ_Sample2D(sampleIdx, launchDim.x, launchDim.x, g_GlobalConstants.m_FrameNumber));
        aoRay.Direction = normalize(normal.xyz + roughness * ranSphere);
        aoRay.Origin = position;
        aoRay.TMax = 8;
        aoRay.TMin = 0.01;

        TraceRay(g_RaytracingTlas, 0, 0xFF, 0, 0, 0, aoRay, payload);
        g_LightingOutput[launchIndex.xy].xyz = ranSphere;

        if (payload.m_Hit)
            light = max(0.1, light - ((1.0 - (payload.m_RayT / aoRay.TMax)) * rcp(NumRays)) * aoIntensity);
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
