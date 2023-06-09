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

#include "engine/world/ecs/components/ecscameracomponent.h"

constexpr uint32_t EcsCameraComponentVersion = 0;

Ether::Ecs::EcsCameraComponent::EcsCameraComponent()
    : EcsToggleComponent(EcsCameraComponentVersion, "Ecs::EcsCameraComponent")
    , m_ProjectionMode(ProjectionMode::Perspective)
    , m_FieldOfView(80.0f)
{
}

void Ether::Ecs::EcsCameraComponent::Serialize(OStream& ostream) const
{
    EcsComponent::Serialize(ostream);

    ostream << static_cast<uint32_t>(m_ProjectionMode);
    ostream << m_FieldOfView;
}

void Ether::Ecs::EcsCameraComponent::Deserialize(IStream& istream)
{
    EcsComponent::Deserialize(istream);

    uint32_t projectionMode;
    istream >> projectionMode;
    m_ProjectionMode = static_cast<ProjectionMode>(projectionMode);
    istream >> m_FieldOfView;
}
