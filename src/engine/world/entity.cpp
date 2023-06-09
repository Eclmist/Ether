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

#include "engine/enginecore.h"
#include "engine/world/entity.h"
#include "engine/world/ecs/components/ecsmetadatacomponent.h"
#include "engine/world/ecs/components/ecstransformcomponent.h"

constexpr uint32_t EntityVersion = 0;

Ether::Entity::Entity()
    : Serializable(EntityVersion, "Engine::Entity")
    , m_EntityID(-1)
    , m_EntityManager(EngineCore::GetActiveWorld().GetEcsManager().GetEntityManager())
    , m_ComponentManager(EngineCore::GetActiveWorld().GetEcsManager().GetComponentManager())
    , m_SystemsManager(EngineCore::GetActiveWorld().GetEcsManager().GetSystemManager())
    , m_SceneGraph(EngineCore::GetActiveWorld().GetSceneGraph())
{
}

Ether::Entity::Entity(const std::string& name, Ecs::EntityID entityID)
    : Serializable(EntityVersion, "Engine::Entity")
    , m_EntityID(entityID)
    , m_EntityManager(EngineCore::GetActiveWorld().GetEcsManager().GetEntityManager())
    , m_ComponentManager(EngineCore::GetActiveWorld().GetEcsManager().GetComponentManager())
    , m_SystemsManager(EngineCore::GetActiveWorld().GetEcsManager().GetSystemManager())
    , m_SceneGraph(EngineCore::GetActiveWorld().GetSceneGraph())
{
    AddComponent<Ecs::EcsMetadataComponent>();
    AddComponent<Ecs::EcsTransformComponent>();

    auto& transform = GetComponent<Ecs::EcsTransformComponent>();
    auto& entityData = GetComponent<Ecs::EcsMetadataComponent>();
    entityData.m_EntityID = m_EntityID;
    entityData.m_EntityName = name;
}

void Ether::Entity::Serialize(OStream& ostream) const
{
    Serializable::Serialize(ostream);
    ostream << m_EntityID;
}

void Ether::Entity::Deserialize(IStream& istream)
{
    Serializable::Deserialize(istream);
    istream >> m_EntityID;
}

std::string Ether::Entity::GetName()
{
    return GetComponent<Ecs::EcsMetadataComponent>().m_EntityName;
}

bool Ether::Entity::IsEnabled()
{
    return GetComponent<Ecs::EcsMetadataComponent>().m_EntityEnabled;
}
