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

#include "engine/world/ecs/component/ecsentitydatacomponent.h"
#include "engine/world/ecs/component/ecstransformcomponent.h"
#include "engine/world/ecs/component/ecsmeshcomponent.h"

Ether::Ecs::EcsManager::EcsManager()
{
    m_ComponentManager.RegisterComponent<EcsEntityDataComponent>();
    m_ComponentManager.RegisterComponent<EcsTransformComponent>();
    m_ComponentManager.RegisterComponent<EcsMeshComponent>();

    m_RenderingSystem = std::make_unique<EcsRenderingSystem>();
    m_SystemManager.RegisterSystem<EcsRenderingSystem>(*m_RenderingSystem);
}

void Ether::Ecs::EcsManager::Update()
{
    m_RenderingSystem->Update();
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

