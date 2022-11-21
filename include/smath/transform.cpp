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

#include "transform.h"

namespace SMath
{
    Transform::Transform()
        : m_TransientTransform(0.0)
        , m_TransientRotation(0.0)
        , m_TransientScale(1.0)
    {
    }

    void Transform::SetTranslation(const Vector3& translation)
    {
        m_TransientTransform = translation;
        UpdateMatrices();
    }

    void Transform::SetRotation(const Vector3& eulerRotation)
    {
        m_TransientRotation = eulerRotation;
        UpdateMatrices();
    }

    void Transform::SetScale(const Vector3& scale)
    {
        m_TransientScale = scale;
        UpdateMatrices();
    }

    void Transform::UpdateMatrices()
    {
        m_Matrix = GetTranslationMatrix(m_TransientTransform) * GetRotationMatrix(m_TransientRotation) * GetScaleMatrix(m_TransientScale);
        m_MatrixInverse = m_Matrix.Inversed();
        m_MatrixTranspose = m_Matrix.Transposed();
        m_MatrixInverseTranspose = m_MatrixInverse.Transposed();
    }

    Vector3 Transform::operator()(const Vector3& v) const
    {
        Vector4 homogeneousVec4(v.x, v.y, v.z, 0);
        Vector4 transformed = m_Matrix * homogeneousVec4;
        return transformed.m_Data;
    }

    Normal3 Transform::operator()(const Normal3& n) const
    {
        Vector4 homogeneousVec4(n.x, n.y, n.z, 0);
        Vector4 transformed = m_MatrixInverseTranspose * homogeneousVec4;
        return transformed.m_Data;
    }

    Point3 Transform::operator()(const Point3& p) const
    {
        Vector4 homogeneousVec4(p.x, p.y, p.z, 1);
        Vector4 transformed = m_Matrix * homogeneousVec4;

        if (transformed.w != 1.0)
            transformed /= transformed.w;

        return transformed.m_Data;
    }

    Ray Transform::operator()(const Ray& r) const
    {
        Point3 origin = r.GetOrigin();
        Vector3 direction = r.GetDirection();
        return Ray((*this)(origin), (*this)(direction));
    }

    Transform Transform::Inversed() const
    {
        Transform inv;
        inv.m_MatrixInverse = m_Matrix;
        inv.m_Matrix = m_MatrixInverse;
        inv.m_MatrixInverseTranspose = m_MatrixTranspose;
        inv.m_MatrixTranspose = m_MatrixInverseTranspose;
        return inv;
    }

    Matrix4x4 Transform::GetTranslationMatrix(const Vector3& translation)
    {
        return { 1.0, 0.0, 0.0, translation.x,
                 0.0, 1.0, 0.0, translation.y,
                 0.0, 0.0, 1.0, translation.z,
                 0.0, 0.0, 0.0, 1.0 };
    }

    Matrix4x4 Transform::GetRotationMatrix(const Vector3& rotation)
    {
        return GetRotationMatrixZ(rotation.z) * GetRotationMatrixY(rotation.y) * GetRotationMatrixX(rotation.x); }

    Matrix4x4 Transform::GetScaleMatrix(const Vector3& scale)
    {
        return { scale.x, 0.0, 0.0, 0.0,
                 0.0, scale.y, 0.0, 0.0,
                 0.0, 0.0, scale.z, 0.0,
                 0.0, 0.0, 0.0, 1.0 };
    }

    Matrix4x4 Transform::GetRotationMatrixX(double rotX)
    {
        return { 1.0, 0.0, 0.0, 0.0,
                 0.0, cos(rotX), -sin(rotX), 0.0,
                 0.0, sin(rotX), cos(rotX), 0.0,
                 0.0, 0.0, 0.0, 1.0 };
    }

    Matrix4x4 Transform::GetRotationMatrixY(double rotY)
    {
        return { cos(rotY), 0.0, sin(rotY), 0.0,
                 0.0, 1.0, 0.0, 0.0,
                 -sin(rotY), 0.0, cos(rotY), 0.0,
                 0.0, 0.0, 0.0, 1.0 };
    }

    Matrix4x4 Transform::GetRotationMatrixZ(double rotZ)
    {
        return { cos(rotZ), -sin(rotZ), 0.0, 0.0,
                 sin(rotZ), cos(rotZ), 0.0, 0.0,
                 0.0, 0.0, 1.0, 0.0,
                 0.0, 0.0, 0.0, 1.0 };
    }

}

