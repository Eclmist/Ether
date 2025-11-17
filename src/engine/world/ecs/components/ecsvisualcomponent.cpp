/*
    This file is part of Ether, an open-source DirectX 12 renderer.

    Copyright (c) 2020-2030 Samuel Huang - All rights reserved.

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

#include "engine/world/ecs/components/ecsvisualcomponent.h"

constexpr uint32_t EcsVisualComponentVersion = 1;
constexpr uint32_t EcsSkinnedVisualComponentVersion = 1;

Ether::Ecs::EcsVisualComponent::EcsVisualComponent()
    : EcsToggleComponent(EcsVisualComponentVersion, "Ecs::EcsVisualComponent")
{
}

void Ether::Ecs::EcsVisualComponent::Serialize(OStream& ostream) const
{
    EcsToggleComponent::Serialize(ostream);
    ostream << m_MeshGuid.GetString();
    ostream << m_MaterialGuid.GetString();
}

void Ether::Ecs::EcsVisualComponent::Deserialize(IStream& istream)
{
    EcsToggleComponent::Deserialize(istream);
    istream >> m_MeshGuid;
    istream >> m_MaterialGuid;
}

Ether::Ecs::EcsSkinnedVisualComponent::EcsSkinnedVisualComponent()
    : EcsToggleComponent(EcsSkinnedVisualComponentVersion, "Ecs::EcsSkinnedVisualComponent")
{
}

void Ether::Ecs::EcsSkinnedVisualComponent::Serialize(OStream& ostream) const
{
    EcsToggleComponent::Serialize(ostream);
    ostream << m_MeshGuid.GetString();
    ostream << m_MaterialGuid.GetString();
    ostream << m_SkeletonGuid.GetString();
    ostream << m_AnimationGuid.GetString();
}

void Ether::Ecs::EcsSkinnedVisualComponent::Deserialize(IStream& istream)
{
    EcsToggleComponent::Deserialize(istream);
    istream >> m_MeshGuid;
    istream >> m_MaterialGuid;
    istream >> m_SkeletonGuid;
    istream >> m_AnimationGuid;
}
