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

#include "engine/world/ecs/ecsentitymanager.h"

Ether::Ecs::EcsEntityManager::EcsEntityManager()
{
    for (EntityID id = 0; id < MaxNumEntities; ++id)
        m_AvailableEntities.push(id);
}

Ether::Ecs::EntityID Ether::Ecs::EcsEntityManager::CreateEntity()
{
    if (m_AvailableEntities.empty())
        throw std::runtime_error("Failed to allocate new entity ID because the max number of entities has been reached");

    EntityID nextID = m_AvailableEntities.front();
    m_AvailableEntities.pop();
    return nextID;
}

void Ether::Ecs::EcsEntityManager::DestroyEntity(EntityID id)
{
    m_AvailableEntities.push(id);
    m_EntitySignatures[id].reset();
}

void Ether::Ecs::EcsEntityManager::SetSignature(EntityID id, EntitySignature signature)
{
    m_EntitySignatures[id] = signature;
}

Ether::Ecs::EntitySignature Ether::Ecs::EcsEntityManager::GetSignature(EntityID id)
{
    return m_EntitySignatures[id];
}

