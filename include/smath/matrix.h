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

namespace SMath
{
    template <typename T, int N>
    class MatrixData
    {
        static_assert(N >= 3 && N <= 4, "Only 3-4 dimensions are supported");
    };

    template <typename T>
    class MatrixData<T, 3>
    {
    public:
        union
        {
            struct { T m_Data[3 * 3]; };
            struct { T m_Data2D[3][3]; };
            struct
            {
                T m_11, m_12, m_13;
                T m_21, m_22, m_23;
                T m_31, m_32, m_33;
            };
        };
    };

    template <typename T>
    class MatrixData<T, 4>
    {
    public:
        union
        {
            struct { T m_Data[4 * 4]; };
            struct { T m_Data2D[4][4]; };
            struct
            {
                T m_11, m_12, m_13, m_14;
                T m_21, m_22, m_23, m_24;
                T m_31, m_32, m_33, m_34;
                T m_41, m_42, m_43, m_44;
            };
        };
    };

    template<typename T, int N>
    class Matrix : public MatrixData<T, N>
    {
    public:
        Matrix();
        Matrix(T v);
        Matrix(const T* data);
        Matrix(T _11, T _12, T _13,
            T _21, T _22, T _23,
            T _31, T _32, T _33);
        Matrix(T _11, T _12, T _13, T _14,
            T _21, T _22, T _23, T _24,
            T _31, T _32, T _33, T _34,
            T _41, T _42, T _43, T _44);

    public:
        inline T operator[](int i) const { return this->m_Data[i]; }
        inline T& operator[](int i) { return this->m_Data[i]; }
        bool operator==(const Matrix& m2) const;
        Matrix operator*(const Matrix& m2) const;

    public:
        bool IsIdentity() const;
        Matrix Transposed() const;
        Matrix<T, 4> Inversed() const;
        Matrix<T, 3> Upper3x3() const;
        
    public:
        static Matrix<T, 4> From3x3(Matrix<T, 3> mat);

    private:
        double Determinant() const;
    };

    #include "matrix_impl.h"

}

