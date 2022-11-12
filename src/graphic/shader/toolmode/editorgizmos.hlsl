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

#include "common/commonconstants.hlsl"
#include "common/samplers.hlsl"

#define AXIS_LINE_WIDTH 1.2
#define AXIS_LINE_ALPHA 1.0
#define GRID_LINE_WIDTH 1.0
#define GRID_LINE_ALPHA 0.7
#define GRID_SIZE 30

struct VS_OUTPUT
{
    float4 Position     : SV_Position;
    float3 PositionWS   : TEXCOORD0;
};

struct GBufferPositionTexIndex
{
    uint m_Index;
};

ConstantBuffer<GBufferPositionTexIndex> m_GBufferPositionTexIndex : register(b1);

float2 GetScreenUV(float2 pos)
{
    return pos / g_CommonConstants.ScreenResolution;
}

VS_OUTPUT VS_Main(uint ID : SV_VertexID)
{
    VS_OUTPUT o;

    float2 pos;
    pos.x = ID % 2 * 2.0 - 1.0;
    pos.y = (ID / 2) * 2.0 - 1.0;

    pos *= length(g_CommonConstants.EyePosition) * GRID_SIZE;

    float4x4 mvp = mul(g_CommonConstants.ProjectionMatrix, g_CommonConstants.ViewMatrix);

    o.PositionWS = float3(pos.x, 0.0, pos.y);
    o.Position = mul(mvp, float4(o.PositionWS.xyz, 1.0));

    return o;
}

float4 GetGridLevel(float level, float3 wPos, float3 dydx)
{
    wPos = wPos * 10 / pow(10.0, level);
    float3 gridWidth = dydx * GRID_LINE_WIDTH * 10 / pow(10.0, level);

    if (frac(wPos.x) < gridWidth.x)
        return float4(0.8, 0.9, 1.0, 0.1 * smoothstep(0.4, 0.0, gridWidth.x)) * GRID_LINE_ALPHA;
    if (frac(wPos.z) < gridWidth.z)
        return float4(0.8, 0.9, 1.0, 0.1 * smoothstep(0.4, 0.0, gridWidth.z)) * GRID_LINE_ALPHA;

    return 0;
}

float4 PS_Main(VS_OUTPUT IN) : SV_Target
{
    Texture2D gBufferPositionTex = ResourceDescriptorHeap[m_GBufferPositionTexIndex.m_Index];
;
    float depthTex = gBufferPositionTex.Sample(g_PointSampler, GetScreenUV(IN.Position.xy)).w;
    float depth = mul(g_CommonConstants.ViewMatrix, float4(IN.PositionWS, 1.0)).z;

    float3 ddxPos = ddx(IN.PositionWS);
    float3 ddyPos = ddy(IN.PositionWS);
    float3 surfaceGradient = abs(ddxPos) + abs(ddyPos);
    float3 axisWidth = surfaceGradient * AXIS_LINE_WIDTH;

    float fadeGradientFactor = 1.0 - saturate(length(surfaceGradient));
    float fadeDepthFactor = smoothstep(0, 0.01, abs(depthTex - depth));
    float fadeAngleFactor = pow(abs(normalize(IN.PositionWS - g_CommonConstants.EyePosition.xyz).y), 0.5);
    float fade = fadeGradientFactor * fadeDepthFactor * fadeAngleFactor;
    float3 axisOpacity = smoothstep(axisWidth, 0, abs(IN.PositionWS)) * AXIS_LINE_ALPHA;

    float4 col = 0;

    // Grid Levels
    for (float i = 0; i < 5; ++i)
        col += GetGridLevel(i, IN.PositionWS, surfaceGradient);

    // Axis lines
    if (axisOpacity.z > 0)
        col = float4(1.0, 0.4, 0.5, axisOpacity.z);
    if (axisOpacity.x > 0)
        col = float4(0.3, 0.7, 1.0, axisOpacity.x);

    col.a *= fade;
    return col;
}
