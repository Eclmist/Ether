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

constexpr uint32_t EcsCameraComponentVersion = 2;

Ether::Ecs::EcsCameraComponent::EcsCameraComponent()
    : EcsToggleComponent(EcsCameraComponentVersion, "Ecs::EcsCameraComponent")
    , m_FieldOfView(80.0f)
    , m_NearPlane(0.01f)
    , m_FarPlane(1000.0f)
    , m_ProjectionMode(ProjectionMode::Perspective)
    , m_JitterMode(JitterMode::None)
{
}

void Ether::Ecs::EcsCameraComponent::Serialize(OStream& ostream) const
{
    EcsComponent::Serialize(ostream);

    ostream << m_FieldOfView;
    ostream << m_NearPlane;
    ostream << m_FarPlane;
    ostream << static_cast<uint32_t>(m_ProjectionMode);
    ostream << static_cast<uint32_t>(m_JitterMode);
}

void Ether::Ecs::EcsCameraComponent::Deserialize(IStream& istream)
{
    EcsComponent::Deserialize(istream);

    istream >> m_FieldOfView;
    istream >> m_NearPlane;
    istream >> m_FarPlane;
    istream >> (uint32_t&)m_ProjectionMode;
    istream >> (uint32_t&)m_JitterMode;
}

Ether::ethVector2 Ether::Ecs::EcsCameraComponent::GetJitterOffset(uint32_t index) const
{
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

Ether::Aabb Ether::Ecs::EcsCameraComponent::GetCameraSpaceFrustum() const
{
    Aabb simpleFrustumCs;
    simpleFrustumCs.m_Min = { -999999.0, -999999.0, m_NearPlane };
    simpleFrustumCs.m_Max = { 999999.0, 999999.0, m_FarPlane };
    return simpleFrustumCs;
}
