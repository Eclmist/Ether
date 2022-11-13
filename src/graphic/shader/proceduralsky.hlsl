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

#include "common/commonconstants.hlsl"
#include "common/fullscreenhelpers.hlsl"
#include "common/samplers.hlsl"

Texture2D EnvironmentHdriTex : register(t0);

#define INV_ATAN float2(0.1591, 0.3183);
#define PI          3.14159265359

float2 SampleSphericalMap(float3 direction)
{
    float2 uv = float2(0.5 + atan2(direction.z,direction.x)/(PI*2), 0.5 - asin(direction.y)/PI);
    uv.x = 1 - uv.x;
    uv.y = 1 - uv.y;

    return uv;
}

struct VS_OUTPUT
{
    float4 Position   : SV_Position;
    float2 UV         : TEXCOORD0;
};

VS_OUTPUT VS_Main(uint ID : SV_VertexID)
{
    float2 pos;
    float2 uv;
    GetVertexFromID(ID, pos, uv);

    VS_OUTPUT o;
    o.Position = float4(pos, 1.0, 1.0);
    o.UV = uv;

    return o;
}

float4 PS_Main(VS_OUTPUT IN) : SV_Target
{
    float skyConst = 0.6;
    float4 zenithColor = float4(0.45, 0.59, 0.95, 1.0) * skyConst;
    float4 horizonColor = float4(205, 220, 253, 255) / 255.0 * skyConst;
    float4 groundColor = float4(0.379, 0.38, 0.74, 1.0)* skyConst;

    float viewY = g_CommonConstants.EyeDirection.y - IN.UV.y + .5;

    float4 col;

//return float4(0.08, 0.06, 0.077, 1.0);
    if (viewY < 0)
        col = lerp( horizonColor, groundColor, saturate(-viewY * 2));
    else
        col = lerp(horizonColor, zenithColor, saturate(viewY * 2.0));

    float2 offset = float2(IN.UV.x - 0.5, 0.5 - IN.UV.y) * g_CommonConstants.ScreenResolution / g_CommonConstants.ScreenResolution.y;

    offset *= 1.39626;

    float3 forward = normalize(g_CommonConstants.EyeDirection.xyz);
    float3 right = normalize(cross(forward, float3(0, 1, 0)));
    float3 up = normalize(cross(right, forward));

    float3 dir = normalize(g_CommonConstants.EyeDirection.xyz - offset.x * right + offset.y * up);
    float2 uv = SampleSphericalMap(dir);
    uv.y = 1-uv.y;
    col = EnvironmentHdriTex.Sample(g_BilinearSampler, uv);

    return col;
}
