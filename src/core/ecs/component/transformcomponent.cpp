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

#include "transformcomponent.h"

ETH_NAMESPACE_BEGIN

// TODO: Should our transform class really be tied to the directxmath library?
using namespace DirectX;

TransformComponent::TransformComponent(EntityID owner)
    : Component(owner)
{
    SetPosition({ 0, 0, 0 });
    SetRotation({ 0, 0, 0 });
    SetScale({ 1, 1, 1 });
}

void TransformComponent::SetPosition(const ethVector3& position)
{
    m_Position = position;
    m_TranslationMatrix = XMMatrixTranslationFromVector(XMLoadFloat3(&position));
    UpdateModelMatrix();
}

void TransformComponent::SetRotation(const ethVector3& eulerRotation)
{
    m_EulerRotation = eulerRotation;
    m_RotationMatrix = XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&eulerRotation));
    UpdateModelMatrix();
}

void TransformComponent::SetScale(const ethVector3& scale)
{
    m_Scale = scale;
    m_ScaleMatrix = XMMatrixScalingFromVector(XMLoadFloat3(&scale));
    UpdateModelMatrix();
}

void TransformComponent::UpdateModelMatrix()
{
    m_ModelMatrix = XMMatrixMultiply(m_RotationMatrix, m_TranslationMatrix);
    m_ModelMatrix = XMMatrixMultiply(m_ScaleMatrix, m_ModelMatrix);
}

ETH_NAMESPACE_END

