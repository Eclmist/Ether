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

#include "transformcomponent.h"

ETH_NAMESPACE_BEGIN

TransformComponent::TransformComponent(EntityID owner)
    : Component(owner)
{
    SetPosition({ 0, 0, 0 });
    SetRotation({ 0, 0, 0 });
    SetScale({ 1, 1, 1 });

	EDITOR_INSPECTOR_FIELD(m_Position, "Position", Vector3Field);
	EDITOR_INSPECTOR_FIELD(m_EulerRotation, "Rotation", Vector3Field);
	EDITOR_INSPECTOR_FIELD(m_Scale, "Scale", Vector3Field);
}

void TransformComponent::Serialize(OStream& ostream)
{
    Component::Serialize(ostream);
}

void TransformComponent::Deserialize(IStream& istream)
{
    Component::Deserialize(istream);
}

void TransformComponent::UpdateMatrices()
{
	m_TranslationMatrix = GetTranslationMatrix(m_Position);
	m_RotationMatrix = GetRotationMatrix(m_EulerRotation);
	m_ScaleMatrix = GetScaleMatrix(m_Scale);

	m_ModelMatrix = m_RotationMatrix * m_TranslationMatrix;
	m_ModelMatrix = m_ScaleMatrix * m_ModelMatrix;
}

ethMatrix4x4 TransformComponent::GetTranslationMatrix(const ethVector3& translation)
{
    return { 1.0f, 0.0f, 0.0f, translation.x,
             0.0f, 1.0f, 0.0f, translation.y,
             0.0f, 0.0f, 1.0f, translation.z,
             0.0f, 0.0f, 0.0f, 1.0f };
}

ethMatrix4x4 TransformComponent::GetRotationMatrix(const ethVector3& eulerRotation)
{
    return GetRotationMatrixZ(eulerRotation.z) * GetRotationMatrixY(eulerRotation.y) * GetRotationMatrixX(eulerRotation.x);
}

ethMatrix4x4 TransformComponent::GetScaleMatrix(const ethVector3& scale)
{
    return { scale.x, 0.0f, 0.0f, 0.0f,
             0.0f, scale.y, 0.0f, 0.0f,
             0.0f, 0.0f, scale.z, 0.0f,
             0.0f, 0.0f, 0.0f, 1.0f };
}
Matrix4x4 TransformComponent::GetRotationMatrixX(float rotX)
{
    return { 1.0f, 0.0f, 0.0f, 0.0f,
             0.0f, cos(rotX), -sin(rotX), 0.0f,
             0.0f, sin(rotX), cos(rotX), 0.0f,
             0.0f, 0.0f, 0.0f, 1.0f };
}

Matrix4x4 TransformComponent::GetRotationMatrixY(float rotY)
{
    return { cos(rotY), 0.0f, sin(rotY), 0.0f,
             0.0f, 1.0f, 0.0f, 0.0f,
             -sin(rotY), 0.0f, cos(rotY), 0.0f,
             0.0f, 0.0f, 0.0f, 1.0f };
}

Matrix4x4 TransformComponent::GetRotationMatrixZ(float rotZ)
{
    return { cos(rotZ), -sin(rotZ), 0.0f, 0.0f,
             sin(rotZ), cos(rotZ), 0.0f, 0.0f,
             0.0f, 0.0f, 1.0f, 0.0f,
             0.0f, 0.0f, 0.0f, 1.0f };
}

ETH_NAMESPACE_END

