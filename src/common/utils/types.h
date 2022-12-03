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

#include "common/common.h"
#include "smath/smath.h"

namespace Ether
{
    using ethVector2    = SMath::Vector<float, 2>;
    using ethVector3    = SMath::Vector<float, 3>;
    using ethVector4    = SMath::Vector<float, 4>;

    using ethVector2i   = SMath::Vector<int, 2>;
    using ethVector3i   = SMath::Vector<int, 3>;
    using ethVector4i   = SMath::Vector<int, 4>;

    using ethVector2u   = SMath::Vector<unsigned int, 2>;
    using ethVector3u   = SMath::Vector<unsigned int, 3>;
    using ethVector4u   = SMath::Vector<unsigned int, 4>;

    using ethPoint2     = SMath::Point<float, 2>;
    using ethPoint3     = SMath::Point<float, 3>;
    using ethPoint4     = SMath::Point<float, 4>;

    using ethPoint2i    = SMath::Point<int, 2>;
    using ethPoint3i    = SMath::Point<int, 3>;
    using ethPoint4i    = SMath::Point<int, 4>;

    using ethPoint2u    = SMath::Point<unsigned int, 2>;
    using ethPoint3u    = SMath::Point<unsigned int, 3>;
    using ethPoint4u    = SMath::Point<unsigned int, 4>;

    using ethMatrix3x3  = SMath::Matrix<float, 3>;
    using ethMatrix4x4  = SMath::Matrix<float, 4>;
    using ethMatrix3x3i = SMath::Matrix<int, 3>;
    using ethMatrix4x4i = SMath::Matrix<int, 4>;
    using ethMatrix3x3u = SMath::Matrix<unsigned int, 3>;
    using ethMatrix4x4u = SMath::Matrix<unsigned int, 4>;

    using Ray           = SMath::Ray;
    using Rect          = SMath::Rect<unsigned int>;
    using Transform     = SMath::Transform;
}
