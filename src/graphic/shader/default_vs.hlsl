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

ConstantBuffer<ModelViewProjection> CB_ModelViewProj : register(b0);

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
    float4 positionOffset = 2.5f * float4(ID % 10, (ID % 100) / 10, ID / 100,0) - float4(30, -5, -5, 0);
    float4 objPos = float4(IN.Position + positionOffset, 1.0f);

    o.Position = mul(CB_ModelViewProj.MVP, objPos);
    o.Color = float4(IN.Color, 1.0f);
    o.Color = float4(sin(objPos.x) / 2 + 0.5, cos(objPos.y) / 2 + 0.5, sin(objPos.z + 2) / 2 + 0.5, 0);

    return o;
}
