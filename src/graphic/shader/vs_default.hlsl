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

struct GlobalConstants
{
    float4 Time;
};

ConstantBuffer<GlobalConstants> CB_GlobalConstants : register(b0);
ConstantBuffer<ModelViewProjection> CB_ModelViewProj : register(b1);

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

    float3 pos = IN.Position;
    //pos.x = -pos.x;
    
    //pos.y += sin(pos.x * CB_GlobalConstants.Time.w) * 0.01;
    //pos.y += sin(pos.z * CB_GlobalConstants.Time.z) * 0.03;

    o.Position = mul(CB_ModelViewProj.ModelViewProjection, float4(pos, 1.0));
    o.PositionES = mul(CB_ModelViewProj.ModelView, float4(IN.Position,1.0)).xyz;
    o.Normal = normalize(mul(CB_ModelViewProj.Normal, normalize(IN.Normal)));
    o.EyeDir = normalize(-o.PositionES);

    return o;
}
