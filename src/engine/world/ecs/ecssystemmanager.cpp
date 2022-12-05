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

#include "engine/world/ecs/ecssystemmanager.h"

#include "engine/world/ecs/systems/ecscamerasystem.h"
#include "engine/world/ecs/systems/ecsvisualsystem.h"

Ether::Ecs::EcsSystemManager::EcsSystemManager()
{
    m_Systems.emplace_back(std::make_unique<EcsCameraSystem>());
    m_Systems.emplace_back(std::make_unique<EcsVisualSystem>());
}

void Ether::Ecs::EcsSystemManager::UpdateEntitySignature(EntityID entityID, EntitySignature newSignature)
{
    for (auto const& system : m_Systems)
    {
        const auto& systemSignature = system->m_Signature;

        if ((newSignature & systemSignature) == systemSignature)
            system->m_Entities.insert(entityID);
        else
            system->m_Entities.erase(entityID);
    }
}

void Ether::Ecs::EcsSystemManager::OnEntityDestroyed(EntityID entityID)
{
    for (auto const& system : m_Systems)
    {
        system->m_Entities.erase(entityID);
    }
}

void Ether::Ecs::EcsSystemManager::Update()
{
    for (auto const& system : m_Systems)
        system->Update();
}

