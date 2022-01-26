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

    float4 EyePosition;
    float4 EyeDirection;
    float4 Time;

    float2 ScreenResolution;
};

ConstantBuffer<CommonConstants> g_CommonConstants : register(b0);
ConstantBuffer<InstanceConstants> g_InstanceConstants : register(b1);

struct VS_INPUT
{
    float3 Position : POSITION;
    float3 Normal   : NORMAL;
    float4 Tangent  : TANGENT0;
    float2 TexCoord : TEXCOORD0;
};

struct VS_OUTPUT
{
    float4 Position     : SV_Position;
    float3 NormalWS     : NORMAL;
    float3 PositionWS   : TEXCOORD0;
};

struct PS_OUTPUT
{
    float4 Albedo       : SV_Target0;
    float4 Normal       : SV_Target1;
    float4 Position     : SV_Target2;
};

VS_OUTPUT VS_Main(VS_INPUT IN, uint ID: SV_InstanceID)
{
    VS_OUTPUT o;

    float trippyAmt = 0.0;

    float3 pos = IN.Position;
    pos.y += sin((pos.x + g_CommonConstants.Time.w) * 7) * 0.5 * trippyAmt;
    pos.y += sin((pos.z + g_CommonConstants.Time.z) * 4) * 0.9 * trippyAmt;

    float4x4 mv = mul(g_CommonConstants.ViewMatrix, g_InstanceConstants.ModelMatrix);
    float4x4 mvp = mul(g_CommonConstants.ProjectionMatrix, mv);

    o.Position = mul(mvp, float4(pos, 1.0));
    o.PositionWS = mul(g_InstanceConstants.ModelMatrix, float4(IN.Position, 1.0)).xyz;
    o.NormalWS = mul(g_InstanceConstants.NormalMatrix, float4(IN.Normal, 1.0)).xyz;

    return o;
}

PS_OUTPUT PS_Main(VS_OUTPUT IN) : SV_Target
{
    float4 col = float4(0.9, 0.9, 0.9, 1.0);
    float3 normal = normalize(IN.NormalWS);
    float3 positionWS = IN.PositionWS;

    float4 col2 = float4(
        sin(positionWS.x) * cos(positionWS.y) * sin(positionWS.z),
        sin(positionWS.x) * cos(positionWS.y) * sin(positionWS.z),
        sin(positionWS.x) * cos(positionWS.y) * sin(positionWS.z),
        1.0);

    float4 col3 = lerp(col, positionWS.y / 10.0, 0.2);// saturate(sin(CB_GlobalConstants.Time.z)));

    PS_OUTPUT output;
    output.Albedo = col3;
    output.Normal = normal.xyzz;
    output.Position.xyz = positionWS.xyz;
    output.Position.w = mul(g_CommonConstants.ViewMatrix, float4(positionWS, 1.0)).z;

    return output;
}