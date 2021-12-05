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

#include "ecsmanager.h"

ETH_NAMESPACE_BEGIN

ECSManager::ECSManager()
{
    m_ComponentManager = std::make_unique<ComponentManager>();
    m_EntityManager = std::make_unique<EntityManager>();
    m_SystemManager = std::make_unique<SystemManager>();
}

void ECSManager::OnInitialize()
{
    m_SystemManager->InitializeSystems();
}

void ECSManager::OnUpdate()
{
    for (auto& system : m_SystemManager->GetSystems())
    {
        system.second->OnUpdate();
    }
}

Entity* ECSManager::GetEntity(EntityID id)
{
    return m_EntityManager->GetEntity(id);
}

Entity* ECSManager::CreateEntity(const std::string& name)
{
    return m_EntityManager->CreateEntity(name);
}

void ECSManager::DestroyEntity(EntityID id)
{
    return m_EntityManager->DestroyEntity(id);
}

#ifdef ETH_TOOLMODE
EntityID ECSManager::GetEntityID(const std::string& guid) const
{
    return m_EntityManager->m_GuidToEntityIDMap[guid];
}
#endif

ETH_NAMESPACE_END

