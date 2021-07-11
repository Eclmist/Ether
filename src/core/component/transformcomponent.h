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
    TransformComponent(Entity* const owner);
    ~TransformComponent() = default;

    inline ethVector3 GetTranslation() const { return m_Translation; }
    inline ethVector3 GetRotation() const { return m_EulerRotation; }
    inline ethVector3 GetScale() const { return m_Scale; }

    void SetTranslation(const ethVector3& translation);
    void SetRotation(const ethVector3& eulerRotation);
    void SetScale(const ethVector3& scale);

    ethXMMatrix GetMatrix();

private:
    ethVector3 m_Translation;
    ethVector3 m_EulerRotation;
    ethVector3 m_Scale;

    ethXMMatrix m_TranslationMatrix;
    ethXMMatrix m_RotationMatrix;
    ethXMMatrix m_ScaleMatrix;
    ethXMMatrix m_FinalMatrix;

    bool m_MatrixRequiresUpdate;
};

ETH_NAMESPACE_END

