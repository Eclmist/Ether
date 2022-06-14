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

#include "component.h"

ETH_NAMESPACE_BEGIN

class ETH_ENGINE_DLL TransformComponent : public Component
{
public:
    TransformComponent(EntityID owner);
    ~TransformComponent() = default;

    void Serialize(OStream& ostream) override;
    void Deserialize(IStream& istream) override;

public:
    inline ethVector3 GetPosition() const { return m_Position; }
    inline ethVector3 GetRotation() const { return m_EulerRotation; }
    inline ethVector3 GetScale() const { return m_Scale; }

    inline void SetPosition(const ethVector3& position) { m_Position = position; }
    inline void SetRotation(const ethVector3& eulerRotation) { m_EulerRotation = eulerRotation; };
    inline void SetScale(const ethVector3& scale) { m_Scale = scale; };

    inline ethMatrix4x4 GetMatrix() const { return m_ModelMatrix; }
    inline const ethMatrix4x4* GetMatrixReference() const { return &m_ModelMatrix; }

public:
    inline std::string GetName() const override { return "Transform"; }

public:
    static ethMatrix4x4 GetTranslationMatrix(const ethVector3& translation);
    static ethMatrix4x4 GetRotationMatrix(const ethVector3& eulerRotation);
    static ethMatrix4x4 GetScaleMatrix(const ethVector3& scale);

    static Matrix4x4 GetRotationMatrixX(float rotX);
    static Matrix4x4 GetRotationMatrixY(float rotY);
    static Matrix4x4 GetRotationMatrixZ(float rotZ);

private:
    friend class TransformationSystem;
    void UpdateMatrices();

private:
    ethVector3 m_Position;
    ethVector3 m_EulerRotation;
    ethVector3 m_Scale;

    ethMatrix4x4 m_TranslationMatrix;
    ethMatrix4x4 m_RotationMatrix;
    ethMatrix4x4 m_ScaleMatrix;

    ethMatrix4x4 m_ModelMatrix;
};

ETH_NAMESPACE_END

