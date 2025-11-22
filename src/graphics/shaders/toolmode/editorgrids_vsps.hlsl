/*
    This file is part of Ether, an open-source DirectX 12 renderer.

    Copyright (c) 2020-2030 Samuel Huang - All rights reserved.

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

#ifndef __EDITOR_GRIDS_VSPS_HLSL__
#define __EDITOR_GRIDS_VSPS_HLSL__

#include "common/globalconstants.h"
#include "utils/constants.hlsl"
#include "utils/helpers.hlsl"

#define AXIS_LINE_WIDTH 1.2
#define AXIS_LINE_ALPHA 1.0
#define GRID_LINE_WIDTH 1.0
#define GRID_LINE_ALPHA 0.7
#define GRID_SIZE 30

Texture2D<float2> g_SceneDepth : register(t0);

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

struct VS_OUTPUT
{
    float4 Position     : SV_Position;
    float3 PositionWS   : TEXCOORD0;
};

VS_OUTPUT VS_Main(uint ID : SV_VertexID)
{
    VS_OUTPUT o;

    uint2 v = uint2(ID / 2, ID % 2);

    float2 pos;
    pos.x = v.x * 2.0f - 1.0f;
    pos.y = v.y * 2.0f - 1.0f;

    pos *= length(g_GlobalConstants.m_CameraPosition) * GRID_SIZE;

    o.PositionWS = float3(pos.x, 0.0, pos.y);
    o.Position = mul(g_GlobalConstants.m_ViewProjectionMatrix, float4(o.PositionWS.xyz, 1.0));

    return o;
}

float4 PS_Main(VS_OUTPUT IN) : SV_Target
{
    const float sceneDepth = g_SceneDepth.Load(float3(IN.Position.xy, 0)).r;
    const float depth = mul(g_GlobalConstants.m_ViewMatrix, float4(IN.PositionWS, 1.0)).z;

    float3 ddxPos = ddx(IN.PositionWS);
    float3 ddyPos = ddy(IN.PositionWS);
    float3 surfaceGradient = abs(ddxPos) + abs(ddyPos);
    float3 axisWidth = surfaceGradient * AXIS_LINE_WIDTH;

    float fadeGradientFactor = 1.0 - saturate(length(surfaceGradient));
    float fadeDepthFactor = smoothstep(0, 0.01, abs(sceneDepth - depth));
    float fadeAngleFactor = pow(abs(normalize(IN.PositionWS - g_GlobalConstants.m_CameraPosition.xyz).y), 0.5);
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

#endif // __EDITOR_GRIDS_VSPS_HLSL__