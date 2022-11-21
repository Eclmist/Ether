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
    typedef SMath::Vector<float, 2>         ethVector2;
    typedef SMath::Vector<float, 3>         ethVector3;
    typedef SMath::Vector<float, 4>         ethVector4;

    typedef SMath::Vector<int, 2>           ethVector2i;
    typedef SMath::Vector<int, 3>           ethVector3i;
    typedef SMath::Vector<int, 4>           ethVector4i;

    typedef SMath::Vector<unsigned int, 2>  ethVector2u;
    typedef SMath::Vector<unsigned int, 3>  ethVector3u;
    typedef SMath::Vector<unsigned int, 4>  ethVector4u;

    typedef SMath::Point<float, 2>          ethPoint2;
    typedef SMath::Point<float, 3>          ethPoint3;
    typedef SMath::Point<float, 4>          ethPoint4;

    typedef SMath::Point<int, 2>            ethPoint2i;
    typedef SMath::Point<int, 3>            ethPoint3i;
    typedef SMath::Point<int, 4>            ethPoint4i;

    typedef SMath::Point<unsigned int, 2>   ethPoint2u;
    typedef SMath::Point<unsigned int, 3>   ethPoint3u;
    typedef SMath::Point<unsigned int, 4>   ethPoint4u;

    typedef SMath::Matrix<float, 3>         ethMatrix3x3;
    typedef SMath::Matrix<float, 4>         ethMatrix4x4;
    typedef SMath::Matrix<int, 3>           ethMatrix3x3i;
    typedef SMath::Matrix<int, 4>           ethMatrix4x4i;
    typedef SMath::Matrix<unsigned int, 3>  ethMatrix3x3u;
    typedef SMath::Matrix<unsigned int, 4>  ethMatrix4x4u;

    typedef SMath::Normal<float, 3>         ethNormal3;

    typedef SMath::Ray                      ethRay;
    typedef SMath::Rect<unsigned int>       ethRect;
    typedef SMath::Transform                ethTransform;
}
