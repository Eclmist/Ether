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
    matrix ModelView;
    matrix ModelViewProjection;
    matrix Normal;
};

struct CommonConstants
{
    float4 LightDirection;
};

struct GlobalConstants
{
    float4 Time;
};

ConstantBuffer<GlobalConstants> CB_GlobalConstants : register(b0);
ConstantBuffer<ModelViewProjection> CB_ModelViewProj : register(b1);
ConstantBuffer<CommonConstants> CB_CommonConstants : register(b2);

struct PS_INPUT
{
    float4 Position : SV_Position;
    float3 Normal   : NORMAL;
    float3 PositionES : TEXCOORD0;
    float3 EyeDir   : TEXCOORD1;
};

float4 PS_Main(PS_INPUT IN) : SV_Target
{
    float3 lightDir = -normalize(CB_CommonConstants.LightDirection).xyz;
    float3 eyeDir = -normalize(IN.PositionES);
    float3 normal = normalize(IN.Normal);
    float4 col = float4(0.9, 0.9, 0.9, 1.0);
    float ndotl = saturate(dot(normal, lightDir));

    float3 r = reflect(-lightDir, normal);
    float rdotv = saturate(dot(r, eyeDir));
    float n = 1000.2;

    float4 ambient = float4(0.4, 0.7, 0.4, 0.1) * 0.3;
    float4 diffuse = col * ndotl;
    float4 specular = col * ndotl * pow(rdotv, n) * 0.5;
   
    //return eyeDir.xyzz;
    return diffuse + specular + ambient;
}

