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

#include "engine/world/ecs/components/ecstransformcomponent.h"

constexpr uint32_t EcsTransformComponentVersion = 0;

Ether::Ecs::EcsTransformComponent::EcsTransformComponent()
    : EcsComponent(EcsTransformComponentVersion, "Ecs::EcsTransformComponent")
    , m_Translation(0.0f, 0.0f, 0.0f)
    , m_Rotation(0.0f, 0.0f, 0.0f)
    , m_Scale(1.0f, 1.0f, 1.0f)
{
}

void Ether::Ecs::EcsTransformComponent::Serialize(OStream& ostream) const
{
    EcsComponent::Serialize(ostream);

    ostream << m_Translation;
    ostream << m_Rotation;
    ostream << m_Scale;
}

void Ether::Ecs::EcsTransformComponent::Deserialize(IStream& istream)
{
    EcsComponent::Deserialize(istream);

    istream >> m_Translation;
    istream >> m_Rotation;
    istream >> m_Scale;
}
