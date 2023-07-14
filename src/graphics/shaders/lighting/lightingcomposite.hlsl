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
#include "utils/fullscreenhelpers.hlsl"

ConstantBuffer<GlobalConstants> g_GlobalConstants   : register(b0);
Texture2D<float4> g_GBufferTexture0                 : register(t0);
Texture2D<float4> g_GBufferTexture1                 : register(t1);
Texture2D<float4> g_GBufferTexture2                 : register(t2);
Texture2D<float4> g_GBufferTexture3                 : register(t3);
Texture2D<float4> g_LightingTexture                 : register(t4);
Texture2D<float4> g_ProceduralSkyTexture            : register(t5);

struct VS_OUTPUT
{
    float4 Position : SV_Position;
    float2 TexCoord : TEXCOORD;
};

VS_OUTPUT VS_Main(uint ID : SV_VertexID)
{
    float2 pos;
    float2 uv;
    GetVertexFromID(ID, pos, uv);

    VS_OUTPUT o;
    o.Position = float4(pos, 1.0, 1.0);
    o.TexCoord = uv;

    return o;
}

float4 PS_Main(VS_OUTPUT IN) : SV_Target
{
    sampler linearSampler = SamplerDescriptorHeap[g_GlobalConstants.m_SamplerIndex_Linear_Wrap];

    float4 lighting = g_LightingTexture.Sample(linearSampler, IN.TexCoord);
    float4 albedo = g_GBufferTexture0[IN.TexCoord * g_GlobalConstants.m_ScreenResolution];
    float4 sky = g_ProceduralSkyTexture[IN.TexCoord * g_GlobalConstants.m_ScreenResolution];

    if (albedo.w == 0)
        return sky;

    if (g_GlobalConstants.m_RaytracedLightingDebug == 1)
        return lighting;

    float4 finalColor = albedo * lighting;
    return finalColor;
}