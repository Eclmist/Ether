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

struct InstanceConstants
{
    float4x4 ModelMatrix;
    float4x4 NormalMatrix;
};

struct CommonConstants
{
    float4x4 ViewMatrix;
    float4x4 ProjectionMatrix;

    float4 EyeDirection;
    float4 Time;
};

ConstantBuffer<CommonConstants> g_CommonConstants : register(b0);
ConstantBuffer<InstanceConstants> g_InstanceConstants : register(b1);

struct VS_INPUT
{
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float4 Tangent : TANGENT0;
    float2 TexCoord : TEXCOORD0;
};

struct VS_OUTPUT
{
    float4 Position : SV_Position;
    float3 Normal   : NORMAL;
    float3 PositionES : TEXCOORD0;
    float3 EyeDir   : TEXCOORD1;
};

VS_OUTPUT VS_Main(VS_INPUT IN, uint ID: SV_InstanceID)
{
    VS_OUTPUT o;

    float4x4 mv = mul(g_InstanceConstants.ModelMatrix, g_CommonConstants.ViewMatrix);
    float4x4 mvp = mul(mv, g_CommonConstants.ProjectionMatrix);

    o.Position = mul(mvp, float4(IN.Position, 1.0));
    o.PositionES = mul(mv, float4(IN.Position,1.0)).xyz;
    o.Normal = normalize(mul(g_InstanceConstants.NormalMatrix, normalize(IN.Normal)));
    o.EyeDir = IN.Position;

    return o;
}

float4 PS_Main(VS_OUTPUT IN) : SV_Target
{
    return float4(1,0,1,1);
}
