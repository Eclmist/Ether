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
Texture2D<float4> g_GBufferAlbedo                   : register(t1);
Texture2D<float4> g_GBufferPosition                 : register(t2);
Texture2D<float4> g_GBufferNormal                   : register(t3);

ConstantBuffer<GlobalConstants> g_GlobalConstants   : register(b0);
RWTexture2D<float4> g_Output                        : register(u0);

sampler g_PointSampler                              : register(s0);

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

    float2 crd = float2(launchIndex.xy);
    float2 dims = launchDim.xy;

    float2 d = ((crd / dims) * 2.f - 1.f);
    float aspectRatio = dims.x / dims.y;

    RayPayload payload;
    payload.m_ScreenPosition = crd;

    float4 albedo = g_GBufferAlbedo.Load(int3(crd, 0));
    float4 position = g_GBufferPosition.Load(int3(crd, 0));
    float4 normal = g_GBufferNormal.Load(int3(crd, 0));

    float4 output = float4(1, 1, 1, 1);

    for (int i = 0; i < 1; ++i)
    {
        RayDesc ray;
        ray.Origin = position.xyz;
        ray.Direction = normalize(normal.xyz + uniformRandomDirection(d + (g_GlobalConstants.m_Time.x * .01 % 4096.) * 27.4723));
        ray.TMin = 0.001;
        ray.TMax = 32;
        TraceRay(g_RaytracingTlas, RAY_FLAG_CULL_BACK_FACING_TRIANGLES, 0xFF, 0, 0, 0, ray, payload);

        if (payload.m_Hit && albedo.w > 0.9)
            output -= (1 - (payload.m_RayT / ray.TMax));
        else
            output = float4(0.3, 0.5, 0.9, 1);
    }

    g_Output[launchIndex.xy] = output;
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
