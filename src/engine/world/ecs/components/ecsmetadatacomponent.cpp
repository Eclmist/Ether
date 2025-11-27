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

#include "engine/world/ecs/components/ecsmetadatacomponent.h"

constexpr uint32_t EcsEntityDataComponentVersion = 0;

// Toolmode specific stuff is being serialized!
// In engine mode, these should go away. But we don't have a process to binarize now,
// so ETH_TOOLMODE checks have been removed so that engine can open scene files for now.

Ether::Ecs::EcsMetadataComponent::EcsMetadataComponent()
    : EcsComponent(EcsEntityDataComponentVersion, "Ecs::EcsEntityDataComponent")
    , m_EntityID(-1)
    , m_EntityName("Entity")
    , m_EntityEnabled(true)
    , m_ToolmodeVisibility(true)
    , m_ToolmodeSelected(false)
{
}

void Ether::Ecs::EcsMetadataComponent::Serialize(OStream& ostream) const
{
    EcsComponent::Serialize(ostream);

    ostream << m_EntityID;
    ostream << m_EntityName;
    ostream << m_EntityEnabled;
    ostream << m_ToolmodeVisibility;
    ostream << m_ToolmodeSelected;
}

void Ether::Ecs::EcsMetadataComponent::Deserialize(IStream& istream)
{
    EcsComponent::Deserialize(istream);

    istream >> m_EntityID;
    istream >> m_EntityName;
    istream >> m_EntityEnabled;
    istream >> m_ToolmodeVisibility;
    istream >> m_ToolmodeSelected;
}
