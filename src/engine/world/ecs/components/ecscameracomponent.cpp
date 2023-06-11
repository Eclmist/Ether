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
#include "graphics/graphiccore.h"

constexpr uint32_t EcsCameraComponentVersion = 1;

Ether::Ecs::EcsCameraComponent::EcsCameraComponent()
    : EcsToggleComponent(EcsCameraComponentVersion, "Ecs::EcsCameraComponent")
    , m_ProjectionMode(ProjectionMode::Perspective)
    , m_JitterMode(JitterMode::None)
    , m_FieldOfView(80.0f)
{
}

void Ether::Ecs::EcsCameraComponent::Serialize(OStream& ostream) const
{
    EcsComponent::Serialize(ostream);

    ostream << static_cast<uint32_t>(m_ProjectionMode);
    ostream << static_cast<uint32_t>(m_JitterMode);
    ostream << m_FieldOfView;
}

void Ether::Ecs::EcsCameraComponent::Deserialize(IStream& istream)
{
    EcsComponent::Deserialize(istream);

    uint32_t projectionMode;
    uint32_t jitterMode;
    istream >> projectionMode;
    istream >> jitterMode;
    m_ProjectionMode = static_cast<ProjectionMode>(projectionMode);
    m_JitterMode = static_cast<JitterMode>(jitterMode);
    istream >> m_FieldOfView;
}

Ether::ethVector2 Ether::Ecs::EcsCameraComponent::GetJitterOffset(uint32_t index)
{
    // TODO: Find a better way to set this
    m_JitterMode = static_cast<JitterMode>(Graphics::GraphicCore::GetGraphicConfig().m_TemporalAAJitterMode);

    switch (m_JitterMode)
    {
    case Ether::Ecs::JitterMode::None:
        return {};
        break;
    case Ether::Ecs::JitterMode::Grid:
        return {};
        break;
    case Ether::Ecs::JitterMode::Halton:
        return GetHaltonSequence<2, 3>(index);
        break;
    default:
        break;
    }

    return {};
}
