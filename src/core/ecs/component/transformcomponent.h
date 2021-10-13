/*
    This file is part of Ether, an open-source DirectX 12 renderer.

    Copyright (c) 2020-2021 Samuel Van Allen - All rights reserved.

    Ether is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include "component.h"

ETH_NAMESPACE_BEGIN

class ETH_ENGINE_DLL TransformComponent : public Component
{
public:
    TransformComponent(EntityID owner);
    ~TransformComponent() = default;

    inline ethVector3 GetPosition() const { return m_Position; }
    inline ethVector3 GetRotation() const { return m_EulerRotation; }
    inline ethVector3 GetScale() const { return m_Scale; }

    void SetPosition(const ethVector3& position);
    void SetRotation(const ethVector3& eulerRotation);
    void SetScale(const ethVector3& scale);

    inline ethXMMatrix GetMatrix() const { return m_ModelMatrix; }
    inline const ethXMMatrix* GetRawMatrix() const { return &m_ModelMatrix; }

public:
    inline std::string GetName() const override { return "Transform"; }

private:
    void UpdateModelMatrix();

private:
    ethVector3 m_Position;
    ethVector3 m_EulerRotation;
    ethVector3 m_Scale;

    ethXMMatrix m_TranslationMatrix;
    ethXMMatrix m_RotationMatrix;
    ethXMMatrix m_ScaleMatrix;

    ethXMMatrix m_ModelMatrix;
};

ETH_NAMESPACE_END

