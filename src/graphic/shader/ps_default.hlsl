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

struct PS_INPUT
{
    float4 Position : SV_Position;
    float4 Normal   : NORMAL;
};

float4 PS_Main(PS_INPUT IN) : SV_Target
{
    float4 lightDir = normalize(mul(CB_ModelViewProj.ModelViewTI, normalize(float4(1., 1., 0., 0.))));// float4(4.0 * sin(CB_GlobalConstants.Time.y), 10, -4, 1.0)));
    float4 col = float4(0.99, 0.99, 0.99, 1.0);
    float ndotl = max(0, dot(IN.Normal.xyz, lightDir.xyz));

    float3 r = reflect(-lightDir, IN.Normal.xyz);
    float3 v = float3(0, 0, -1);
    float rdotv = max(0, dot(r, v));
    float n = 2.0;

    float4 ambient = float4(0.0, 0.1, 0.0, 0.1);

    return (col * ndotl + col * pow(rdotv, n) * 3.0 + ambient * 1.5);
}

