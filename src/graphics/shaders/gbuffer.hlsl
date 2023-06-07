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
    float4 PositionWS   : TEXCOORD1;
    float4 PositionPrev : TEXCOORD2;
};

struct PS_OUTPUT
{
    float4 Output0      : SV_TARGET0;
    float4 Output1      : SV_TARGET1;
    float4 Output2      : SV_TARGET2;
    float4 DebugOutput  : SV_TARGET3;
};

ConstantBuffer<GlobalConstants> g_GlobalConstants : register(b0);
ConstantBuffer<Material> g_InstanceParams : register(b1);

VS_OUTPUT VS_Main(VS_INPUT IN)
{
    VS_OUTPUT o;

    float3 pos = IN.Position;

    //float4x4 mv = mul(g_GlobalConstants.m_ViewMatrix, m_InstanceParams.m_ModelMatrix); (just leave model matrix as identity for now)
    float4x4 mvp = mul(g_GlobalConstants.m_ProjectionMatrix, g_GlobalConstants.m_ViewMatrix);
    float4x4 mvpPrev = mul(g_GlobalConstants.m_ProjectionMatrixPrev, g_GlobalConstants.m_ViewMatrixPrev);

    o.Position = mul(mvp, float4(pos, 1.0f));
    o.PositionPrev = mul(mvpPrev, float4(pos, 1.0f));
    o.PositionWS = /* mul(m_ModelMatrix, */ float4(pos, 1.0f); //).xyz;
    //o.NormalWS = mul(m_InstanceParams.m_NormalMatrix, float4(IN.Normal, 1.0f)).xyz;

    o.TexCoord = IN.Tangent.xy;
    o.Normal = IN.Normal;

    return o;
}

PS_OUTPUT PS_Main(VS_OUTPUT IN) : SV_Target
{
    //float2 a = (IN.PositionSS.xy / IN.PositionSS.w) * 0.5 + 0.5;
    float2 posCurr = IN.Position.xy;
   // posCurr.x = g_GlobalConstants.m_ScreenResolution.x - posCurr.x;

    float2 prevPos = ((IN.PositionPrev.xy * float2(1, -1) / IN.PositionPrev.w) * 0.5 + 0.5) * g_GlobalConstants.m_ScreenResolution;
    //prevPos.y = 1 - prevPos.y;

    float2 velocity = posCurr - prevPos;

    PS_OUTPUT o;
    o.Output0 = float4(g_InstanceParams.m_BaseColor.xyz, 1);
    o.Output1 = float4(IN.PositionWS.xyz, IN.Normal.x);
    o.Output2 = float4(IN.Normal.yz, velocity);
    //o.DebugOutput = float4(o.Output2.zw, 0, 0);
    o.DebugOutput = float4(1 / IN.Position.www / 1, 0);
    return o;
}