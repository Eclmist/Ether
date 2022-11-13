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

#pragma once
#include "math/math.h"

ETH_NAMESPACE_BEGIN

typedef Matrix4x4                       ethMatrix4x4;
typedef Matrix3x3                       ethMatrix3x3;

typedef Vector<float, 2>                ethVector2;
typedef Vector<float, 3>                ethVector3;
typedef Vector<float, 4>                ethVector4;
typedef Vector<int, 2>                  ethVector2i;
typedef Vector<int, 3>                  ethVector3i;
typedef Vector<int, 4>                  ethVector4i;
typedef Vector<uint32_t, 2>             ethVector2u;
typedef Vector<uint32_t, 3>             ethVector3u;
typedef Vector<uint32_t, 4>             ethVector4u;

ETH_NAMESPACE_END
