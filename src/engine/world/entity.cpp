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
#include "engine/world/ecs/component/ecsentitydatacomponent.h"
#include "engine/world/ecs/component/ecstransformcomponent.h"

Ether::Entity::Entity(const std::string& name)
    : m_EntityID(EngineCore::GetActiveWorld().GetEcsManager().CreateEntity())
    , m_EntityManager(EngineCore::GetActiveWorld().GetEcsManager().GetEntityManager())
    , m_ComponentManager(EngineCore::GetActiveWorld().GetEcsManager().GetComponentManager())
    , m_SystemsManager(EngineCore::GetActiveWorld().GetEcsManager().GetSystemManager())
    , m_SceneGraph(EngineCore::GetActiveWorld().GetSceneGraph())
{
    AddComponent<Ecs::EcsEntityDataComponent>();
    AddComponent<Ecs::EcsTransformComponent>();

    auto entityData = GetComponent<Ecs::EcsEntityDataComponent>();
    entityData.m_EntityID = m_EntityID;
    entityData.m_EntityName = name;
    entityData.m_EntityEnabled = true;

    auto transform = GetComponent<Ecs::EcsTransformComponent>();
    transform.m_Translation = { 0, 0, 0 };
    transform.m_Rotation = { 0, 0, 0 };
    transform.m_Scale = { 1, 1, 1 };
}

std::string Ether::Entity::GetName()
{
    return GetComponent<Ecs::EcsEntityDataComponent>().m_EntityName;
}

bool Ether::Entity::IsEnabled()
{
    return GetComponent<Ecs::EcsEntityDataComponent>().m_EntityEnabled;
}

