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

#include "vector.h"

namespace SMath
{
    template<typename T, int N>
    class Normal : public Vector<T, N>
    {
    public:
        Normal(T v = 0.0);
        Normal(T x, T y);
        Normal(T x, T y, T z);
        Normal(T x, T y, T z, T w);
        Normal(const T* data);
        Normal(const Normal& v);
        Normal(const Vector<T, N>& v);
    };

    #include "normal_impl.h" 
}

