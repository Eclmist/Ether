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
    matrix MVP;
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
    float3 Color    : COLOR;
};

struct VS_OUTPUT
{
    float4 Position : SV_Position;
    float4 Color    : COLOR;
};

VS_OUTPUT VS_Main(VS_INPUT IN, uint ID: SV_InstanceID)
{
    VS_OUTPUT o;
    float offset = sin(CB_GlobalConstants.Time.x) + sin(CB_GlobalConstants.Time.z + 0.2);
    offset += cos(CB_GlobalConstants.Time.z + 0.4);
    offset += cos(CB_GlobalConstants.Time.z + 0.5);
    offset += cos(CB_GlobalConstants.Time.w + 0.9);
    offset += cos(CB_GlobalConstants.Time.y + 1.4);
    offset += cos(CB_GlobalConstants.Time.x + 9.4);
    float4 positionOffset = (float4(ID % 10, (ID % 100) / 10, ID / 100, 0) - 4.5) * 3;
    positionOffset -= float4(offset * ((ID % 4) + 1), offset * ((ID % 5) + 1), offset * ((ID%6) + 1) * 0.7, 0) * 0.01;
    float4 objPos = float4(IN.Position + positionOffset, 1.0f);

    o.Position = mul(CB_ModelViewProj.MVP, objPos);
    o.Color = float4(IN.Color, 1.0f);
    o.Color = float4(sin(objPos.x + CB_GlobalConstants.Time.z) / 2 + 0.5, cos(objPos.y + CB_GlobalConstants.Time.x) / 2 + 0.5, sin(objPos.z + 2) / 2 + 0.5, 0);

    return o;
}
