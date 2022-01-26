/*
    This file is part of Ether, an open-source DirectX 12 renderer.

    Copyright (c) 2020-2022 Samuel Huang - All rights reserved.

    Ether is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

//#include "common/fullscreenhelpers.hlsl"

void GetVertexFromID(const uint vertexID, out float2 pos, out float2 uv)
{
    uint2 v = uint2(vertexID % 2, vertexID / 2);

    pos.x = v.x * 2.0 - 1.0;
    pos.y = v.y * 2.0 - 1.0;

    uv.x = v.x;
    uv.y = 1.0 - v.y;
}

struct VS_OUTPUT
{
    float4 Position   : SV_Position;
    float2 UV         : TEXCOORD0;
};

struct CommonConstants
{
    float4x4 ViewMatrix;
    float4x4 ProjectionMatrix;

    float4 EyePosition;
    float4 EyeDirection;
    float4 Time;

    float2 ScreenResolution;
};

ConstantBuffer<CommonConstants> g_CommonConstants : register(b0);

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

return float4(0.07, 0.06, 0.077, 1.0);
    if (viewY < 0)
        col = lerp( horizonColor, groundColor, saturate(-viewY * 2));
    else
        col = lerp(horizonColor, zenithColor, saturate(viewY * 2.0));

    return col;
}
