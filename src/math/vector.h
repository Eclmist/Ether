/*
    This file is part of Ether, an open-source DirectX 12 renderer.

    Copyright (c) 2020-2022 Samuel Huang - All rights reserved.

    Ether is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <type_traits>

template <typename T, int N>
class Vector;

template <typename T, int N>
class VectorData
{
    static_assert(N >= 2 && N <= 4, "Only 2-4 dimensions are supported");
};

template <typename T>
class VectorData<T, 2>
{
public:
    union
    {
        struct { T x, y; };
        struct { T m_Data[2]; };
    };
};

template <typename T>
class VectorData<T, 3>
{
public:
    union
    {
        struct { T x, y, z; };
        struct { T m_Data[3]; };
    };
};

template <typename T>
class VectorData<T, 4>
{
public:
    union
    {
        struct { T x, y, z, w; };
        struct { T m_Data[4]; };
    };
};

template<typename T, int N>
class Vector : public VectorData<T, N>
{
    static_assert(std::is_arithmetic_v<T>, "Vector only works with arithmetic types");

public:
    Vector(T v = 0.0);
    Vector(T x, T y);
    Vector(T x, T y, T z);
    Vector(T x, T y, T z, T w);
    Vector(T* data);

    Vector operator+() const;
    Vector operator-() const;

    Vector operator+(const Vector& b) const;
    Vector operator-(const Vector& b) const;
    Vector operator*(const Vector& b) const;
    Vector operator/(const Vector& b) const;

    Vector& operator+=(const Vector& b);
    Vector& operator-=(const Vector& b);
    Vector& operator*=(const Vector& b);
    Vector& operator/=(const Vector& b);

    bool operator==(const Vector& b) const;
    bool operator!=(const Vector& b) const;

    inline T operator[](int i) const { return this->m_Data[i]; }
    inline T& operator[](int i) { return this->m_Data[i]; }

public:
    double Magnitude() const;
    T SquareMagnitude() const;
    void Normalize();
    Vector Normalized() const;

public:
    static T Dot(const Vector& a, const Vector& b);
    static T AbsDot(const Vector& a, const Vector& b);
    static double Angle(const Vector& a, const Vector& b);
    static double CosAngle(const Vector& a, const Vector& b);
    static Vector<T, 3> Cross(const Vector& a, const Vector& b);
};

typedef Vector<double, 4> Vector4;
typedef Vector<double, 3> Vector3;
typedef Vector<double, 2> Vector2;

typedef Vector<int, 2> Vector2i;
typedef Vector<int, 3> Vector3i;
typedef Vector<int, 4> Vector4i;

#include "vector_impl.h" 