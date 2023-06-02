/*
    This file is part of Ether, an open-source DirectX 12 renderer.

    Copyright (c) 2020-2023 Samuel Huang - All rights reserved.

    Ether is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

struct GlobalConstants
{
    float4x4 m_ViewMatrix;
    float4x4 m_ProjectionMatrix;

    float4 m_EyePosition;
    float4 m_EyeDirection;
    float4 m_Time;

    uint2 m_ScreenResolution;
};

ConstantBuffer<GlobalConstants> g_GlobalConstants : register(b0);
