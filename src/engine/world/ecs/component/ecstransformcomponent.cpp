/*
    This file is part of Ether, an open-source DirectX 12 renderer.

    Copyright (c) 2020-2023 Samuel Huang - All rights reserved.

    Ether is free software: you can redistribute it and/or modify
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

#include "engine/world/ecs/component/ecstransformcomponent.h"

constexpr uint32_t EcsTransformComponentVersion = 0;

Ether::Ecs::EcsTransformComponent::EcsTransformComponent()
    : EcsIndexedComponent(EcsTransformComponentVersion, StringID("Ecs::EcsTransformComponent").GetHash())
{
}

void Ether::Ecs::EcsTransformComponent::Serialize(OStream& ostream)
{
    Serializable::Serialize(ostream);

    ostream << m_Translation;
    ostream << m_Rotation;
    ostream << m_Scale;
}

void Ether::Ecs::EcsTransformComponent::Deserialize(IStream& istream)
{
    Serializable::Deserialize(istream);

    istream >> m_Translation;
    istream >> m_Rotation;
    istream >> m_Scale;
}
