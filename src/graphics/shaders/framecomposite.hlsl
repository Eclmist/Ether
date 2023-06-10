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
#include "common/framecompositeinputs.h"
#include "utils/fullscreenhelpers.hlsl"

ConstantBuffer<GlobalConstants> g_GlobalConstants : register(b0);
ConstantBuffer<FrameCompositeInputs> g_FrameCompositeInputs : register(b1);
sampler g_BilinearSampler : register(s0);

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
    Texture2D<float4> lightingOutput = ResourceDescriptorHeap[g_FrameCompositeInputs.m_LightingTextureIndex];
    float4 light = lightingOutput[IN.TexCoord * g_GlobalConstants.m_ScreenResolution];

    Texture2D<float4> gbufferTex0 = ResourceDescriptorHeap[g_FrameCompositeInputs.m_GBufferTextureIndex0];
    float4 albedo = gbufferTex0.Sample(g_BilinearSampler, IN.TexCoord);

    float4 finalColor = albedo * light;

    if (albedo.w == 0)
        return 1;

    return finalColor;
}