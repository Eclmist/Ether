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

RaytracingAccelerationStructure g_RaytracingTlas : register(t0);
RWTexture2D<float4> g_Output : register(u0);

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
    float3 col = linearToSrgb(float3(0.4, 0.6, 0.2));
    g_Output[launchIndex.xy] = float4(col, 1);
}

struct Payload
{
    bool hit;
};

[shader("miss")]
void Miss(inout Payload payload)
{ 
    payload.hit = false;
}

[shader("closesthit")]
void ClosestHit(inout Payload payload, in BuiltInTriangleIntersectionAttributes attribs)
{
    payload.hit = true;
}
