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

struct PS_INPUT
{
    float4 Position : SV_Position;
    float4 Color    : COLOR;
};

float4 PS_Main(PS_INPUT IN, uint pid : SV_PrimitiveID) : SV_Target
{
    //return float4(sin(IN.Position.x) / 2 + 0.5, cos(IN.Position.y) / 2 + 0.5, sin(IN.Position.z + 2) / 2 + 0.5, 0);
    //return float4(pid / 12.0f, pid / 12.0 + 0.3f, pid / 12.0 + 0.6f, 1.0);
    return IN.Color;
}

