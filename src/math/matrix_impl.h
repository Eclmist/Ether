#include "matrix.h"
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

template<typename T, int N>
Matrix<T, N>::Matrix(T v)
{
    for (int i = 0; i < N * N; ++i)
        this->m_Data[i] = v;
}

template<typename T, int N>
Matrix<T, N>::Matrix(const T* data)
{
    for (int i = 0; i < N * N; ++i)
        this->m_Data[i] = data[i];
}

template<typename T, int N>
Matrix<T, N>::Matrix(
    T _11, T _12, T _13, 
    T _21, T _22, T _23,
    T _31, T _32, T _33)
{
    static_assert(N == 3, "9 component constructor only available for N = 3");
    this->m_11 = _11;
    this->m_12 = _12;
    this->m_13 = _13;
    this->m_21 = _21;
    this->m_22 = _22;
    this->m_23 = _23;
    this->m_31 = _31;
    this->m_32 = _32;
    this->m_33 = _33;
}

template<typename T, int N>
Matrix<T, N>::Matrix(
    T _11, T _12, T _13, T _14,
    T _21, T _22, T _23, T _24,
    T _31, T _32, T _33, T _34,
    T _41, T _42, T _43, T _44)
{
    static_assert(N == 4, "16 component constructor only available for N = 4");
    this->m_11 = _11;
    this->m_12 = _12;
    this->m_13 = _13;
    this->m_14 = _14;
    this->m_21 = _21;
    this->m_22 = _22;
    this->m_23 = _23;
    this->m_24 = _24;
    this->m_31 = _31;
    this->m_32 = _32;
    this->m_33 = _33;
    this->m_34 = _34;
    this->m_41 = _41;
    this->m_42 = _42;
    this->m_43 = _43;
    this->m_44 = _44;
}

template<typename T, int N>
bool Matrix<T, N>::operator==(const Matrix& m2) const
{
    for (int i = 0; i < N*N; ++i)
        if (m_Data[i] != m2.m_Data[i])
            return false;

    return true;
}

template<typename T, int N>
Matrix<T, N> Matrix<T, N>::operator*(const Matrix& m2) const
{
    float data[N][N] = { 0 };

    for (int i = 0; i < N; ++i)
    {
        for (int j = 0; j < N; ++j)
        {
            data[i][j] = 0;
            for (int k = 0; k < N; ++k)
                data[i][j] += m_Data2D[i][k] * m2.m_Data2D[k][j];
        }
    }

    return &(data[0][0]);
}

template<typename T, int N>
bool Matrix<T, N>::IsIdentity() const
{
    for (int i = 0; i < N; ++i)
        for (int j = 0; j < N; ++j)
        {
            if (i == j && this->m_Data2D[i][j] != 1)
                return false;

            if (i != j && this->m_Data2D[i][j] != 0)
                return false;
        }

    return true;
}

template<typename T, int N>
Matrix<T, N> Matrix<T, N>::Transposed() const
{
    Matrix<T, N> transposed;

    for (int i = 0; i < N; ++i)
        for (int j = 0; j < N; ++j)
            transposed.m_Data2D[i][j] = this->m_Data2D[j][i];

    return transposed;
}

template<typename T, int N>
Matrix<T, 4> Matrix<T, N>::Inversed() const
{
    float d = Determinant();
    if (fabs(d) < 0.001)
        return Matrix4x4();

    // use Cramer's rule
    d = 1.0f / d;
    Matrix4x4 out;
    out.m_Data[0] = d * (m_Data[5] * (m_Data[10] * m_Data[15] - m_Data[11] * m_Data[14]) + m_Data[6] * (m_Data[11] * m_Data[13] - m_Data[9] * m_Data[15]) + m_Data[7] * (m_Data[9] * m_Data[14] - m_Data[10] * m_Data[13]));
    out.m_Data[1] = d * (m_Data[9] * (m_Data[2] * m_Data[15] - m_Data[3] * m_Data[14]) + m_Data[10] * (m_Data[3] * m_Data[13] - m_Data[1] * m_Data[15]) + m_Data[11] * (m_Data[1] * m_Data[14] - m_Data[2] * m_Data[13]));
    out.m_Data[2] = d * (m_Data[13] * (m_Data[2] * m_Data[7] - m_Data[3] * m_Data[6]) + m_Data[14] * (m_Data[3] * m_Data[5] - m_Data[1] * m_Data[7]) + m_Data[15] * (m_Data[1] * m_Data[6] - m_Data[2] * m_Data[5]));
    out.m_Data[3] = d * (m_Data[1] * (m_Data[7] * m_Data[10] - m_Data[6] * m_Data[11]) + m_Data[2] * (m_Data[5] * m_Data[11] - m_Data[7] * m_Data[9]) + m_Data[3] * (m_Data[6] * m_Data[9] - m_Data[5] * m_Data[10]));
    out.m_Data[4] = d * (m_Data[6] * (m_Data[8] * m_Data[15] - m_Data[11] * m_Data[12]) + m_Data[7] * (m_Data[10] * m_Data[12] - m_Data[8] * m_Data[14]) + m_Data[4] * (m_Data[11] * m_Data[14] - m_Data[10] * m_Data[15]));
    out.m_Data[5] = d * (m_Data[10] * (m_Data[0] * m_Data[15] - m_Data[3] * m_Data[12]) + m_Data[11] * (m_Data[2] * m_Data[12] - m_Data[0] * m_Data[14]) + m_Data[8] * (m_Data[3] * m_Data[14] - m_Data[2] * m_Data[15]));
    out.m_Data[6] = d * (m_Data[14] * (m_Data[0] * m_Data[7] - m_Data[3] * m_Data[4]) + m_Data[15] * (m_Data[2] * m_Data[4] - m_Data[0] * m_Data[6]) + m_Data[12] * (m_Data[3] * m_Data[6] - m_Data[2] * m_Data[7]));
    out.m_Data[7] = d * (m_Data[2] * (m_Data[7] * m_Data[8] - m_Data[4] * m_Data[11]) + m_Data[3] * (m_Data[4] * m_Data[10] - m_Data[6] * m_Data[8]) + m_Data[0] * (m_Data[6] * m_Data[11] - m_Data[7] * m_Data[10]));
    out.m_Data[8] = d * (m_Data[7] * (m_Data[8] * m_Data[13] - m_Data[9] * m_Data[12]) + m_Data[4] * (m_Data[9] * m_Data[15] - m_Data[11] * m_Data[13]) + m_Data[5] * (m_Data[11] * m_Data[12] - m_Data[8] * m_Data[15]));
    out.m_Data[9] = d * (m_Data[11] * (m_Data[0] * m_Data[13] - m_Data[1] * m_Data[12]) + m_Data[8] * (m_Data[1] * m_Data[15] - m_Data[3] * m_Data[13]) + m_Data[9] * (m_Data[3] * m_Data[12] - m_Data[0] * m_Data[15]));
    out.m_Data[10] = d * (m_Data[15] * (m_Data[0] * m_Data[5] - m_Data[1] * m_Data[4]) + m_Data[12] * (m_Data[1] * m_Data[7] - m_Data[3] * m_Data[5]) + m_Data[13] * (m_Data[3] * m_Data[4] - m_Data[0] * m_Data[7]));
    out.m_Data[11] = d * (m_Data[3] * (m_Data[5] * m_Data[8] - m_Data[4] * m_Data[9]) + m_Data[0] * (m_Data[7] * m_Data[9] - m_Data[5] * m_Data[11]) + m_Data[1] * (m_Data[4] * m_Data[11] - m_Data[7] * m_Data[8]));
    out.m_Data[12] = d * (m_Data[4] * (m_Data[10] * m_Data[13] - m_Data[9] * m_Data[14]) + m_Data[5] * (m_Data[8] * m_Data[14] - m_Data[10] * m_Data[12]) + m_Data[6] * (m_Data[9] * m_Data[12] - m_Data[8] * m_Data[13]));
    out.m_Data[13] = d * (m_Data[8] * (m_Data[2] * m_Data[13] - m_Data[1] * m_Data[14]) + m_Data[9] * (m_Data[0] * m_Data[14] - m_Data[2] * m_Data[12]) + m_Data[10] * (m_Data[1] * m_Data[12] - m_Data[0] * m_Data[13]));
    out.m_Data[14] = d * (m_Data[12] * (m_Data[2] * m_Data[5] - m_Data[1] * m_Data[6]) + m_Data[13] * (m_Data[0] * m_Data[6] - m_Data[2] * m_Data[4]) + m_Data[14] * (m_Data[1] * m_Data[4] - m_Data[0] * m_Data[5]));
    out.m_Data[15] = d * (m_Data[0] * (m_Data[5] * m_Data[10] - m_Data[6] * m_Data[9]) + m_Data[1] * (m_Data[6] * m_Data[8] - m_Data[4] * m_Data[10]) + m_Data[2] * (m_Data[4] * m_Data[9] - m_Data[5] * m_Data[8]));

    return out;
}

template<typename T, int N>
float Matrix<T, N>::Determinant() const
{
    return
        (m_Data[0] * m_Data[5] - m_Data[1] * m_Data[4]) * (m_Data[10] * m_Data[15] - m_Data[11] * m_Data[14]) -
        (m_Data[0] * m_Data[6] - m_Data[2] * m_Data[4]) * (m_Data[9] * m_Data[15] - m_Data[11] * m_Data[13]) +
        (m_Data[0] * m_Data[7] - m_Data[3] * m_Data[4]) * (m_Data[9] * m_Data[14] - m_Data[10] * m_Data[13]) +
        (m_Data[1] * m_Data[6] - m_Data[2] * m_Data[5]) * (m_Data[8] * m_Data[15] - m_Data[11] * m_Data[12]) -
        (m_Data[1] * m_Data[7] - m_Data[3] * m_Data[5]) * (m_Data[8] * m_Data[14] - m_Data[10] * m_Data[12]) +
        (m_Data[2] * m_Data[7] - m_Data[3] * m_Data[6]) * (m_Data[8] * m_Data[13] - m_Data[9] * m_Data[12]);
}

template<typename T, int N>
Matrix<T, 4> Matrix<T, N>::From3x3(Matrix<T, 3> mat)
{
    return {
        mat.m_11, mat.m_12, mat.m_13, 0,
        mat.m_21, mat.m_22, mat.m_23, 0,
        mat.m_31, mat.m_32, mat.m_33, 0,
        0, 0, 0, 1
    };
}

template<typename T, int N>
Matrix<T, 3> Matrix<T, N>::Upper3x3() const
{
    return {
        m_11, m_12, m_13,
        m_21, m_22, m_23,
        m_31, m_32, m_33
    };
}

