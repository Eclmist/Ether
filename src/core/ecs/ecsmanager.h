/*
    This file is part of Ether, an open-source DirectX 12 renderer.

    Copyright (c) 2020-2021 Samuel Van Allen - All rights reserved.

    Ether is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <bitset>

ETH_NAMESPACE_BEGIN

// Arbitrarily defined max constants. This value will affect the 
// allocated memory for systems. 
#define ETH_ECS_MAX_ENTITIES 4096
#define ETH_ECS_MAX_COMPONENTS 32
#define ETH_ECS_ROOT_ENTITY_ID (ETH_ECS_MAX_ENTITIES)
#define ETH_ECS_INVALID_ID -1

using EntityID = uint32_t;
using ComponentID = uint32_t;
using ComponentSignature = std::bitset<ETH_ECS_MAX_COMPONENTS>;

ETH_NAMESPACE_END

#include "core/ecs/entity/entitymanager.h"
#include "core/ecs/component/componentmanager.h"
#include "core/ecs/component/transformcomponent.h"
#include "core/ecs/component/meshcomponent.h"
#include "core/ecs/component/visualcomponent.h"
#include "core/ecs/system/systemmanager.h"
#include "core/ecs/system/renderingsystem.h"

ETH_NAMESPACE_BEGIN

class ETH_ENGINE_DLL ECSManager : public NonCopyable
{
public:
    ECSManager();
    ~ECSManager() = default;

public:
    void OnInitialize();
    void OnUpdate();

public:
    Entity* GetEntity(EntityID id);
    Entity* CreateEntity(const std::string& name);
    void DestroyEntity(EntityID id);

public:
    template <typename T>
    T* AddComponent(EntityID id)
    {
        T* component = m_ComponentManager->AddComponent<T>(id);

        Entity* entity = m_EntityManager->GetEntity(id);
        entity->SetComponentSignature(GetComponentID<T>() , true);
        m_SystemManager->AssignEntityToSystems(id, entity->GetComponentSignature());

        return component;
    }

    template <typename T>
    void RemoveComponent(EntityID id)
    {
        m_ComponentManager->RemoveComponent<T>(id);
        Entity* entity = m_EntityManager->GetEntity(id);
        entity->SetComponentSignature(GetComponentID<T>(), false);
        m_SystemManager->AssignEntityToSystems(id, entity->GetComponentSignature());
    }

    template <typename T>
    T* GetComponent(EntityID id)
    {
        return m_ComponentManager->GetComponent<T>(id);
    }

    template <typename T>
    std::shared_ptr<ComponentArray<T>> GetComponents()
    {
        return m_ComponentManager->GetComponent<T>();
    }

    template <typename T>
    ComponentID GetComponentID()
    {
        return m_ComponentManager->GetComponentID<T>();
    }

private:
    std::unique_ptr<ComponentManager> m_ComponentManager;
    std::unique_ptr<EntityManager> m_EntityManager;
    std::unique_ptr<SystemManager> m_SystemManager;
};

ETH_NAMESPACE_END

