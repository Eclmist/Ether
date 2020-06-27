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

#pragma once

// Use DXMath classes for SIMD/SSE optimized types
#include <DirectXMath.h>

typedef DirectX::XMMATRIX           ethMatrix;
typedef DirectX::XMFLOAT2           ethVector2;
typedef DirectX::XMFLOAT3           ethVector3;
typedef DirectX::XMFLOAT4           ethVector4;
typedef DirectX::XMUINT2            ethVector2u;
typedef DirectX::XMUINT3            ethVector3u;
typedef DirectX::XMUINT4            ethVector4u;
typedef DirectX::XMINT2             ethVector2i;
typedef DirectX::XMINT3             ethVector3i;
typedef DirectX::XMINT4             ethVector4i;
