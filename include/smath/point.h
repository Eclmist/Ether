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
    class Point : public VectorData<T, N>
    {
        static_assert(std::is_arithmetic_v<T>, "Vector only works with arithmetic types");

    public:
        Point(T v = 0.0);
        Point(T x, T y);
        Point(T x, T y, T z);
        Point(T x, T y, T z, T w);
        Point(const T* data);

        Point operator+() const;
        Point operator-() const;

        bool operator==(const Point& b) const;
        bool operator!=(const Point& b) const;

        inline T operator[](int i) const { return this->m_Data[i]; }
        inline T& operator[](int i) { return this->m_Data[i]; }

    public:
        static double Distance(const Point& a, const Point& b);
        static double SquareDistance(const Point& a, const Point& b);
    };

    #include "point_impl.h" 
}

