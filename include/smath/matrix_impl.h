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

template<typename T, int N>
inline Matrix<T, N>::Matrix()
{
    for (int i = 0; i < N; ++i)
        for (int j = 0; j < N; ++j)
            this->m_Data2D[i][j] = i == j ? 1 : 0;
}

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
    for (int i = 0; i < N * N; ++i)
        if (std::fabs(this->m_Data[i] - m2.m_Data[i]) > SMath::Epsilon)
            return false;

    return true;
}

template<typename T, int N>
Matrix<T, N> Matrix<T, N>::operator*(const Matrix& m2) const
{
    T data[N][N];

    for (int i = 0; i < N; ++i)
    {
        for (int j = 0; j < N; ++j)
        {
            data[i][j] = 0;
            for (int k = 0; k < N; ++k)
                data[i][j] += this->m_Data2D[i][k] * m2.m_Data2D[k][j];
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
            if (i == j && std::fabs(this->m_Data2D[i][j] - 1) > SMath::Epsilon)
                return false;

            if (i != j && std::fabs(this->m_Data2D[i][j]) > SMath::Epsilon)
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
    double d = Determinant();
    if (std::fabs(d) < 0.001)
        return Matrix<T, 4>();

    // use Cramer's rule
    d = 1.0f / d;
    Matrix<T, 4> out;
    out.m_Data[0] = d * (this->m_Data[5] * (this->m_Data[10] * this->m_Data[15] - this->m_Data[11] * this->m_Data[14]) + this->m_Data[6] * (this->m_Data[11] * this->m_Data[13] - this->m_Data[9] * this->m_Data[15]) + this->m_Data[7] * (this->m_Data[9] * this->m_Data[14] - this->m_Data[10] * this->m_Data[13]));
    out.m_Data[1] = d * (this->m_Data[9] * (this->m_Data[2] * this->m_Data[15] - this->m_Data[3] * this->m_Data[14]) + this->m_Data[10] * (this->m_Data[3] * this->m_Data[13] - this->m_Data[1] * this->m_Data[15]) + this->m_Data[11] * (this->m_Data[1] * this->m_Data[14] - this->m_Data[2] * this->m_Data[13]));
    out.m_Data[2] = d * (this->m_Data[13] * (this->m_Data[2] * this->m_Data[7] - this->m_Data[3] * this->m_Data[6]) + this->m_Data[14] * (this->m_Data[3] * this->m_Data[5] - this->m_Data[1] * this->m_Data[7]) + this->m_Data[15] * (this->m_Data[1] * this->m_Data[6] - this->m_Data[2] * this->m_Data[5]));
    out.m_Data[3] = d * (this->m_Data[1] * (this->m_Data[7] * this->m_Data[10] - this->m_Data[6] * this->m_Data[11]) + this->m_Data[2] * (this->m_Data[5] * this->m_Data[11] - this->m_Data[7] * this->m_Data[9]) + this->m_Data[3] * (this->m_Data[6] * this->m_Data[9] - this->m_Data[5] * this->m_Data[10]));
    out.m_Data[4] = d * (this->m_Data[6] * (this->m_Data[8] * this->m_Data[15] - this->m_Data[11] * this->m_Data[12]) + this->m_Data[7] * (this->m_Data[10] * this->m_Data[12] - this->m_Data[8] * this->m_Data[14]) + this->m_Data[4] * (this->m_Data[11] * this->m_Data[14] - this->m_Data[10] * this->m_Data[15]));
    out.m_Data[5] = d * (this->m_Data[10] * (this->m_Data[0] * this->m_Data[15] - this->m_Data[3] * this->m_Data[12]) + this->m_Data[11] * (this->m_Data[2] * this->m_Data[12] - this->m_Data[0] * this->m_Data[14]) + this->m_Data[8] * (this->m_Data[3] * this->m_Data[14] - this->m_Data[2] * this->m_Data[15]));
    out.m_Data[6] = d * (this->m_Data[14] * (this->m_Data[0] * this->m_Data[7] - this->m_Data[3] * this->m_Data[4]) + this->m_Data[15] * (this->m_Data[2] * this->m_Data[4] - this->m_Data[0] * this->m_Data[6]) + this->m_Data[12] * (this->m_Data[3] * this->m_Data[6] - this->m_Data[2] * this->m_Data[7]));
    out.m_Data[7] = d * (this->m_Data[2] * (this->m_Data[7] * this->m_Data[8] - this->m_Data[4] * this->m_Data[11]) + this->m_Data[3] * (this->m_Data[4] * this->m_Data[10] - this->m_Data[6] * this->m_Data[8]) + this->m_Data[0] * (this->m_Data[6] * this->m_Data[11] - this->m_Data[7] * this->m_Data[10]));
    out.m_Data[8] = d * (this->m_Data[7] * (this->m_Data[8] * this->m_Data[13] - this->m_Data[9] * this->m_Data[12]) + this->m_Data[4] * (this->m_Data[9] * this->m_Data[15] - this->m_Data[11] * this->m_Data[13]) + this->m_Data[5] * (this->m_Data[11] * this->m_Data[12] - this->m_Data[8] * this->m_Data[15]));
    out.m_Data[9] = d * (this->m_Data[11] * (this->m_Data[0] * this->m_Data[13] - this->m_Data[1] * this->m_Data[12]) + this->m_Data[8] * (this->m_Data[1] * this->m_Data[15] - this->m_Data[3] * this->m_Data[13]) + this->m_Data[9] * (this->m_Data[3] * this->m_Data[12] - this->m_Data[0] * this->m_Data[15]));
    out.m_Data[10] = d * (this->m_Data[15] * (this->m_Data[0] * this->m_Data[5] - this->m_Data[1] * this->m_Data[4]) + this->m_Data[12] * (this->m_Data[1] * this->m_Data[7] - this->m_Data[3] * this->m_Data[5]) + this->m_Data[13] * (this->m_Data[3] * this->m_Data[4] - this->m_Data[0] * this->m_Data[7]));
    out.m_Data[11] = d * (this->m_Data[3] * (this->m_Data[5] * this->m_Data[8] - this->m_Data[4] * this->m_Data[9]) + this->m_Data[0] * (this->m_Data[7] * this->m_Data[9] - this->m_Data[5] * this->m_Data[11]) + this->m_Data[1] * (this->m_Data[4] * this->m_Data[11] - this->m_Data[7] * this->m_Data[8]));
    out.m_Data[12] = d * (this->m_Data[4] * (this->m_Data[10] * this->m_Data[13] - this->m_Data[9] * this->m_Data[14]) + this->m_Data[5] * (this->m_Data[8] * this->m_Data[14] - this->m_Data[10] * this->m_Data[12]) + this->m_Data[6] * (this->m_Data[9] * this->m_Data[12] - this->m_Data[8] * this->m_Data[13]));
    out.m_Data[13] = d * (this->m_Data[8] * (this->m_Data[2] * this->m_Data[13] - this->m_Data[1] * this->m_Data[14]) + this->m_Data[9] * (this->m_Data[0] * this->m_Data[14] - this->m_Data[2] * this->m_Data[12]) + this->m_Data[10] * (this->m_Data[1] * this->m_Data[12] - this->m_Data[0] * this->m_Data[13]));
    out.m_Data[14] = d * (this->m_Data[12] * (this->m_Data[2] * this->m_Data[5] - this->m_Data[1] * this->m_Data[6]) + this->m_Data[13] * (this->m_Data[0] * this->m_Data[6] - this->m_Data[2] * this->m_Data[4]) + this->m_Data[14] * (this->m_Data[1] * this->m_Data[4] - this->m_Data[0] * this->m_Data[5]));
    out.m_Data[15] = d * (this->m_Data[0] * (this->m_Data[5] * this->m_Data[10] - this->m_Data[6] * this->m_Data[9]) + this->m_Data[1] * (this->m_Data[6] * this->m_Data[8] - this->m_Data[4] * this->m_Data[10]) + this->m_Data[2] * (this->m_Data[4] * this->m_Data[9] - this->m_Data[5] * this->m_Data[8]));

    return out;
}

template<typename T, int N>
double Matrix<T, N>::Determinant() const
{
    return
        (this->m_Data[0] * this->m_Data[5] - this->m_Data[1] * this->m_Data[4]) * (this->m_Data[10] * this->m_Data[15] - this->m_Data[11] * this->m_Data[14]) -
        (this->m_Data[0] * this->m_Data[6] - this->m_Data[2] * this->m_Data[4]) * (this->m_Data[9] * this->m_Data[15] - this->m_Data[11] * this->m_Data[13]) +
        (this->m_Data[0] * this->m_Data[7] - this->m_Data[3] * this->m_Data[4]) * (this->m_Data[9] * this->m_Data[14] - this->m_Data[10] * this->m_Data[13]) +
        (this->m_Data[1] * this->m_Data[6] - this->m_Data[2] * this->m_Data[5]) * (this->m_Data[8] * this->m_Data[15] - this->m_Data[11] * this->m_Data[12]) -
        (this->m_Data[1] * this->m_Data[7] - this->m_Data[3] * this->m_Data[5]) * (this->m_Data[8] * this->m_Data[14] - this->m_Data[10] * this->m_Data[12]) +
        (this->m_Data[2] * this->m_Data[7] - this->m_Data[3] * this->m_Data[6]) * (this->m_Data[8] * this->m_Data[13] - this->m_Data[9] * this->m_Data[12]);
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
        this->m_11, this->m_12, this->m_13,
        this->m_21, this->m_22, this->m_23,
        this->m_31, this->m_32, this->m_33
    };
}

