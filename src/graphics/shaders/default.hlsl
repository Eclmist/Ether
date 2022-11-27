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

struct Material
{
    float4 m_BaseColor;
    float4 m_SpecularColor;
    float m_Roughness;
    float m_Metalness;
};

struct VS_OUTPUT
{
    float4 Position : SV_Position;
    float2 TexCoord : TEXCOORD0;
};

ConstantBuffer<Material> m_InstanceParams : register(b0);

VS_OUTPUT VS_Main(uint ID : SV_VertexID, uint INSTANCE : SV_InstanceID)
{
    VS_OUTPUT o;

    float2 pos;
    float2 uv;
    GetVertexFromID(ID, pos, uv);

    pos /= 4.0f;
    pos.x += m_InstanceParams.m_Metalness;

    o.Position = float4(pos, 1.0f, 1.0f);

    o.TexCoord = uv;

    return o;
}

float4 PS_Main(VS_OUTPUT IN) : SV_Target
{
    float d = 0.1f;

    //for (int i = 0; i < 1000000; ++i)
    //    d += i;


    return float4(IN.TexCoord, m_InstanceParams.m_Roughness, d);
}

