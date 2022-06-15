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

#include "visualcomponent.h"

ETH_NAMESPACE_BEGIN

VisualComponent::VisualComponent(EntityID owner)
    : Component(owner)
{
    m_Material = std::make_shared<Material>();

	EDITOR_INSPECTOR_FIELD(m_Material->m_BaseColor, "Base Color", Vector4Field);
	EDITOR_INSPECTOR_FIELD(m_Material->m_SpecularColor, "Specular Color", Vector4Field);
    EDITOR_INSPECTOR_RANGE_FIELD(m_Material->m_Roughness, "Roughness", FloatField, 0.0f, 1.0f);
    EDITOR_INSPECTOR_RANGE_FIELD(m_Material->m_Metalness, "Metalness", FloatField, 0.0f, 1.0f);
}

VisualComponent::~VisualComponent()
{
}

void VisualComponent::Serialize(OStream& ostream)
{
    Component::Serialize(ostream);
}

void VisualComponent::Deserialize(IStream& istream)
{
    Component::Deserialize(istream);
}

ETH_NAMESPACE_END


