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
#include "common/globalconstants.hlsl"

struct Material
{
    float4 m_BaseColor;
    float4 m_SpecularColor;
    float m_Roughness;
    float m_Metalness;
};

struct VS_INPUT
{
    float3 Position     : POSITION;
    float3 Normal       : NORMAL;
    float3 Tangent      : TANGENT;
    float3 BiTangent    : BITANGENT;
    float2 TexCoord     : TEXCOORD;
};

struct VS_OUTPUT
{
    float4 Position     : SV_POSITION;
    float3 Normal       : NORMAL;
    float2 TexCoord     : TEXCOORD0;
};

ConstantBuffer<Material> m_InstanceParams : register(b1);

VS_OUTPUT VS_Main(VS_INPUT IN)
{
    VS_OUTPUT o;

    float3 pos = IN.Position;

    //float4x4 mv = mul(g_GlobalConstants.m_ViewMatrix, m_InstanceParams.m_ModelMatrix); (just leave model matrix as identity for now)
    float4x4 mvp = mul(g_GlobalConstants.m_ProjectionMatrix, g_GlobalConstants.m_ViewMatrix);

    o.Position = mul(mvp, float4(pos, 1.0f));
    //o.PositionWS = mul(m_InstanceParams.m_ModelMatrix, float4(pos, 1.0f)).xyz;
    //o.NormalWS = mul(m_InstanceParams.m_NormalMatrix, float4(IN.Normal, 1.0f)).xyz;

    o.TexCoord = IN.Tangent.xy;
    o.Normal = IN.Normal;

    return o;
}

float4 PS_Main(VS_OUTPUT IN) : SV_Target
{
    return float4(IN.Normal, 1.0f);
}

