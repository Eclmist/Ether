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

#ifndef __EDITOR_OUTLINE_PS_HLSL__
#define __EDITOR_OUTLINE_PS_HLSL__

#include "common/globalconstants.h"
#include "utils/constants.hlsl"
#include "utils/helpers.hlsl"

float4 PS_Main(float4 : SV_Position) : SV_Target
{
    return float4(1, 0.5, 0.1, 1);
}

#endif // __EDITOR_OUTLINE_PS_HLSL__