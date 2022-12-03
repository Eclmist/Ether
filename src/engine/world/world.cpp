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

#include "engine/world/world.h"

constexpr uint32_t WorldVersion = 0;

Ether::World::World()
    : Serializable(WorldVersion, StringID("Engine::World").GetHash())
{
}

void Ether::World::Update()
{
    m_EcsManager.Update();
}

void Ether::World::Serialize(OStream& ostream)
{
    Serializable::Serialize(ostream);
    ostream << m_WorldName;
    m_SceneGraph.Serialize(ostream);
}

void Ether::World::Deserialize(IStream& istream)
{
    Serializable::Deserialize(istream);
    istream >> m_WorldName;
    m_SceneGraph.Deserialize(istream);
}

Ether::Entity& Ether::World::CreateEntity(const std::string& name)
{
    std::unique_ptr<Entity> entity = std::make_unique<Entity>(name);
    Ecs::EntityID entityID = entity->GetID();
    m_Entities[entityID] = std::move(entity);
    return *m_Entities[entityID];
}

