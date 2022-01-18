/*
    This file is part of Ether, an open-source DirectX 12 renderer.

    Copyright (c) 2020-2021 Samuel Van Allen - All rights reserved.

    Ether is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

struct ModelViewProjection
{
    matrix ModelMatrix;
    matrix ModelViewProjection;
    matrix Normal;
};

struct GlobalConstants
{
    float4 Time;
};

ConstantBuffer<GlobalConstants> CB_GlobalConstants : register(b0);
ConstantBuffer<ModelViewProjection> CB_ModelViewProj : register(b1);

struct PS_INPUT
{
    float4 Position   : SV_Position;
    float3 NormalWS   : NORMAL;
    float3 PositionWS : TEXCOORD0;
};

struct PS_OUTPUT
{
    float4 Albedo       : SV_Target0;
    float4 Normal       : SV_Target1;
    float4 Position     : SV_Target2;
};

PS_OUTPUT PS_Main(PS_INPUT IN) : SV_Target
{
    float4 col = float4(0.3, 0.9, 0.9, 1.0);
    float3 normal = normalize(IN.NormalWS);
    float3 positionWS = IN.PositionWS;

    PS_OUTPUT output;
    output.Albedo = col;
    output.Normal = normal.xyzz / 2.0 + 0.5;
    output.Position = positionWS.xyzz;

    return output;
}
