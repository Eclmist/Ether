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

// TODO: Should our transform class really be tied to the directxmath library?
using namespace DirectX;

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
    // TODO
}

void TransformComponent::Deserialize(IStream& istream)
{
    Component::Deserialize(istream);
    // TODO
}

void TransformComponent::UpdateMatrices()
{
	m_TranslationMatrix = XMMatrixTranslationFromVector(XMLoadFloat3(&m_Position));
	m_RotationMatrix = XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&m_EulerRotation));
	m_ScaleMatrix = XMMatrixScalingFromVector(XMLoadFloat3(&m_Scale));

	m_ModelMatrix = XMMatrixMultiply(m_RotationMatrix, m_TranslationMatrix);
	m_ModelMatrix = XMMatrixMultiply(m_ScaleMatrix, m_ModelMatrix);
}

ETH_NAMESPACE_END

