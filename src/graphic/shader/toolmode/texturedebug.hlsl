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

#include "common/fullscreenhelpers.hlsl"
#include "common/samplers.hlsl"

struct VS_OUTPUT
{
    float4 Position : SV_Position;
    float2 UV       : TEXCOORD0;
};

struct TextureDebugParam
{
    uint m_TextureIndex;
};

ConstantBuffer<TextureDebugParam> m_TextureDebugParam : register(b1);

VS_OUTPUT VS_Main(uint ID : SV_VertexID)
{
    float2 pos;
    float2 uv;
    GetVertexFromID(ID, pos, uv);

    VS_OUTPUT o;
    o.Position = float4(pos, 0.0, 1.0);
    o.UV = uv;

    return o;
}

float4 PS_Main(VS_OUTPUT IN) : SV_Target
{
    Texture2D debugTex = ResourceDescriptorHeap[m_TextureDebugParam.m_TextureIndex];
    return debugTex.Sample(g_PointSampler, IN.UV);
}
