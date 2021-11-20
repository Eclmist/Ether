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
    matrix ModelViewTI;
    matrix ModelViewProjection;
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
    float4 Normal   : NORMAL;
};

VS_OUTPUT VS_Main(VS_INPUT IN, uint ID: SV_InstanceID)
{
    VS_OUTPUT o;

    o.Position = mul(CB_ModelViewProj.ModelViewProjection, float4(IN.Position, 1.0));
    o.Normal = mul(CB_ModelViewProj.ModelViewTI, normalize(float4(IN.Normal, 1.0)));

    return o;
}
