/*
    This file is part of SMath, an open-source math library for graphics
    applications.

    Copyright (c) 2020-2023 Samuel Van Allen - All rights reserved.

    Spectre is free software: you can redistribute it and/or modify
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

#include "linalg.h"
#include "ray.h"

namespace SMath
{
    template <typename T>
    class Transform
    {
    public:
        static Matrix<T, 4> GetTranslationMatrix(const Vector<T, 3>& translation);
        static Matrix<T, 4> GetRotationMatrix(const Vector<T, 3>& rotation);
        static Matrix<T, 4> GetScaleMatrix(const Vector<T, 3>& scale);

        static Matrix<T, 4> GetRotationMatrixX(T rotX);
        static Matrix<T, 4> GetRotationMatrixY(T rotY);
        static Matrix<T, 4> GetRotationMatrixZ(T rotZ);

        static Matrix<T, 4> GetPerspectiveMatrixLH(T fovy, T aspect, T znear, T zfar);
        static Matrix<T, 4> GetPerspectiveMatrixRH(T fovy, T aspect, T znear, T zfar);
    };

#include "transform_impl.h"
}

