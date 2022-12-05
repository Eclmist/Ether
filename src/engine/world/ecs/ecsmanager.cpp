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

#include "engine/world/ecs/ecsmanager.h"

constexpr uint32_t EcsManagerVersion = 0;

Ether::Ecs::EcsManager::EcsManager()
    : Serializable(EcsManagerVersion, StringID("Engine::EcsManager").GetHash())
{
}

void Ether::Ecs::EcsManager::Serialize(OStream& ostream) const
{
    Serializable::Serialize(ostream);
    m_EntityManager.Serialize(ostream);
    m_ComponentManager.Serialize(ostream);
}

void Ether::Ecs::EcsManager::Deserialize(IStream& istream)
{
    Serializable::Deserialize(istream);
    m_EntityManager.Deserialize(istream);
    m_ComponentManager.Deserialize(istream);
}

void Ether::Ecs::EcsManager::Update()
{
    ETH_MARKER_EVENT("Ecs Manager - Update");
    m_SystemManager.Update();
}

Ether::Ecs::EntityID Ether::Ecs::EcsManager::CreateEntity()
{
    return m_EntityManager.CreateEntity();
}

void Ether::Ecs::EcsManager::DestroyEntity(EntityID entityID)
{
    m_EntityManager.DestroyEntity(entityID);
    m_SystemManager.OnEntityDestroyed(entityID);
    m_ComponentManager.OnEntityDestroyed(entityID);
}

