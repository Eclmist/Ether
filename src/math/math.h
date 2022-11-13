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

#include "matrix.h"
#include "vector.h"
#include <limits>

ETH_NAMESPACE_BEGIN

#define ETH_M_PI                3.141592653589793f
#define ETH_M_INVPI             0.31830988618f
#define ETH_M_INV2PI            0.15915494309f
#define ETH_M_INV4PI            0.07957747154f
#define ETH_M_PIOVER2           1.57079632679f
#define ETH_M_PIOVER4           0.78539816339f

#define ETH_EPSILON             0.00001f
#define ETH_ONE_MINUS_EPSILON   0.99999f

#define ethSaturate(x)          ethClamp(x, 0, 1)
#define ethPow(x, n)            pow(x, n)
#define ethDegToRad(x)          x * (ETH_M_PI / 180.0f)


static constexpr float MaxFloat = ::std::numeric_limits<float>::max();
static constexpr float MinFloat = ::std::numeric_limits<float>::min();
static constexpr float Infinity = ::std::numeric_limits<float>::infinity();

template <typename _Type>
inline _Type ethMax(_Type lhs, _Type rhs)
{
    return lhs >= rhs ? lhs : rhs;
}

template <typename _Type>
inline _Type ethMin(_Type lhs, _Type rhs)
{
    return lhs <= rhs ? lhs : rhs;
}

template <typename _Type1, typename _Type2, typename _Type3>
inline _Type1 ethClamp(_Type1 v, _Type2 a, _Type3 b)
{
    return v < a ? a : (v < b ? v : b);
}

template <typename _Type1, typename _Type2>
inline _Type1 ethLerp(_Type1 a, _Type1 b, _Type2 t)
{
    return ((static_cast<_Type2>(1) - t) * a + t * b);
}

ETH_NAMESPACE_END

