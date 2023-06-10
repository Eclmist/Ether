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

template <typename T>
SMath::Matrix<T, 4> SMath::Transform<T>::GetTranslationMatrix(const Vector<T, 3>& translation)
{
    return { 1, 0, 0, translation.x,
             0, 1, 0, translation.y,
             0, 0, 1, translation.z,
             0, 0, 0, 1 };
}

template <typename T>
SMath::Matrix<T, 4> SMath::Transform<T>::GetRotationMatrix(const Vector<T, 3>& rotation)
{
    return GetRotationMatrixZ(rotation.z) * GetRotationMatrixY(rotation.y) * GetRotationMatrixX(rotation.x);
}

template <typename T>
SMath::Matrix<T, 4> SMath::Transform<T>::GetScaleMatrix(const Vector<T, 3>& scale)
{
    return { scale.x, 0, 0, 0,
             0, scale.y, 0, 0,
             0, 0, scale.z, 0,
             0, 0, 0, 1 };
}

template <typename T>
SMath::Matrix<T, 4> SMath::Transform<T>::GetRotationMatrixX(T rotX)
{
    return { 1, 0, 0, 0,
             0, cos(rotX), -sin(rotX), 0,
             0, sin(rotX), cos(rotX), 0,
             0, 0, 0, 1 };
}

template <typename T>
SMath::Matrix<T, 4> SMath::Transform<T>::GetRotationMatrixY(T rotY)
{
    return { cos(rotY), 0, sin(rotY), 0,
             0, 1, 0, 0,
             -sin(rotY), 0, cos(rotY), 0,
             0, 0, 0, 1 };
}

template <typename T>
SMath::Matrix<T, 4> SMath::Transform<T>::GetRotationMatrixZ(T rotZ)
{
    return { cos(rotZ), -sin(rotZ), 0, 0,
             sin(rotZ), cos(rotZ), 0, 0,
             0, 0, 1, 0,
             0, 0, 0, 1 };
}

template <typename T>
SMath::Matrix<T, 4> SMath::Transform<T>::GetPerspectiveMatrixLH(T fovy, T aspect, T znear, T zfar)
{
    T f = 1.0 / tan(fovy / 2.0);

    Matrix<T, 4> projection;
    projection.m_Data2D[0][0] = f / aspect;
    projection.m_Data2D[1][1] = f;
    projection.m_Data2D[2][2] = zfar / (zfar - znear);
    projection.m_Data2D[3][2] = 1.0;
    projection.m_Data2D[2][3] = -znear * zfar / (zfar - znear);
    projection.m_Data2D[3][3] = 0.0;

    return projection;
}

template <typename T>
SMath::Matrix<T, 4> SMath::Transform<T>::GetPerspectiveMatrixRH(T fovy, T aspect, T znear, T zfar)
{
    T f = 1.0 / tan(fovy / 2.0);

    Matrix<T, 4> projection;
    projection.m_Data2D[0][0] = f / aspect;
    projection.m_Data2D[1][1] = f;
    projection.m_Data2D[2][2] = zfar / (znear - zfar);
    projection.m_Data2D[3][2] = -1.0;
    projection.m_Data2D[2][3] = znear * zfar / (znear - zfar);
    projection.m_Data2D[3][3] = 0.0;

    return projection;
}

