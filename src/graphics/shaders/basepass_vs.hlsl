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

#ifndef __BASEPASS_VS_HLSL__
#define __BASEPASS_VS_HLSL__

#include "common/globalconstants.h"
#include "common/material.h"
#include "common/instanceparams.h"
#include "utils/encoding.hlsl"
#include "utils/noise.hlsl"
#include "utils/shading.hlsl"

struct VS_INPUT
{
    float3 Position         : POSITION;
    float3 Normal           : NORMAL;
    float3 Tangent          : TANGENT;
    float4 Color            : COLOR;
    float2 TexCoord         : TEXCOORD0;
    float3 PositionPrev     : TEXCOORD1;
};

struct VS_OUTPUT
{
    float4 Position         : SV_POSITION;
    float3 Normal           : NORMAL;
    float4 Color            : COLOR;
    float3 Tangent          : TEXCOORD0;
    float2 TexCoord         : TEXCOORD1;
    float4 ClipPos          : TEXCOORD2;
    float4 ClipPosPrev      : TEXCOORD3;
};

ConstantBuffer<InstanceParams> InstanceParams : register(b1);

VS_OUTPUT VS_Main(VS_INPUT IN)
{
    VS_OUTPUT o;

    const float4 worldPos = mul(InstanceParams.m_ModelMatrix, float4(IN.Position, 1.0f));
    const float4 worldPosPrev = mul(InstanceParams.m_ModelMatrixPrev, float4(IN.PositionPrev, 1.0f));
    
    o.Position = mul(GlobalConstants.m_ViewProjectionMatrix, worldPos);
    o.Normal = mul(InstanceParams.m_NormalMatrix, float4(IN.Normal, 0.0f)).xyz;
    o.Tangent = IN.Tangent;
    o.TexCoord = IN.TexCoord;
    o.Color = IN.Color;

    o.ClipPos = mul(GlobalConstants.m_ViewProjectionMatrixNoJitter, worldPos);
    o.ClipPosPrev = mul(GlobalConstants.m_ViewProjectionMatrixPrevNoJitter, worldPosPrev);

    return o;
}

#endif // __BASEPASS_VS_HLSL__