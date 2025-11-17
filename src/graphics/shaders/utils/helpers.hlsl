/*
    This file is part of Ether, an open-source DirectX 12 renderer.

    Copyright (c) 2020-2030 Samuel Huang - All rights reserved.

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

#ifndef __HELPERS_HLSL__
#define __HELPERS_HLSL__

#include "common/globalconstants.h"

float GetLuminanceFromRGB(float3 rgb)
{
    return dot(float3(0.2126, 0.7152, 0.0722), rgb);
}

float LinearizeDepth(float depth)
{
    float near = g_GlobalConstants.m_CameraClipNearFar.x;
    float far = g_GlobalConstants.m_CameraClipNearFar.y;
    return far * near / (depth * (far - near) + near);
}

#endif // __HELPERS_HLSL__
