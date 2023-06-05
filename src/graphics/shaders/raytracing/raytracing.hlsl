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

#include "common/globalconstants.hlsl"

RaytracingAccelerationStructure g_RaytracingTlas    : register(t0);
ConstantBuffer<GlobalConstants> g_GlobalConstants   : register(b0);
RWTexture2D<float4> g_Output                        : register(u0);

struct Payload
{
    float3 color;
};

float3 linearToSrgb(float3 c)
{
    // Based on http://chilliant.blogspot.com/2012/08/srgb-approximations-for-hlsl.html
    float3 sq1 = sqrt(c);
    float3 sq2 = sqrt(sq1);
    float3 sq3 = sqrt(sq2);
    float3 srgb = 0.662002687 * sq1 + 0.684122060 * sq2 - 0.323583601 * sq3 - 0.0225411470 * c;
    return srgb;
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
    RayDesc ray;

    ray.Origin = g_GlobalConstants.m_EyePosition.xyz;
    ray.Direction = mul(transpose(g_GlobalConstants.m_ViewMatrix), normalize(float3(d.x * aspectRatio, -d.y, 1.2)));
    ray.TMin = 0;
    ray.TMax = 100000;

    Payload payload;
    TraceRay(g_RaytracingTlas, RAY_FLAG_CULL_BACK_FACING_TRIANGLES, 0xFF, 0 /* ray index*/, 0, 0, ray, payload);
    float3 col = linearToSrgb(payload.color);
    g_Output[launchIndex.xy] = float4(col, 1);
}

[shader("miss")]
void Miss(inout Payload payload)
{ 
    payload.color = float3(1,0,1);
}

[shader("closesthit")]
void ClosestHit(inout Payload payload, in BuiltInTriangleIntersectionAttributes attribs)
{
    float3 barycentrics = float3(
        1.0 - attribs.barycentrics.x - attribs.barycentrics.y,
        attribs.barycentrics.x,
        attribs.barycentrics.y
    );

    const float3 R = float3(1, 0, 0);
    const float3 G = float3(0, 1, 0);
    const float3 B = float3(0, 0, 1);

    payload.color = R * barycentrics.x + G * barycentrics.y + B * barycentrics.z;
}
