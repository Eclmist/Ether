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

#include "entitymanager.h"

ETH_NAMESPACE_BEGIN

EntityManager::EntityManager()
{
    for (EntityID id = 0; id < ETH_MAX_ENTITIES; ++id)
        m_AvailableEntityIDs.push(id);
}

EntityManager::~EntityManager()
{
}

Entity* EntityManager::CreateEntity(const std::string& name)
{
    EntityID newID = ReserveNextEntityID();

    if (newID == ETH_INVALID_ENTITYID)
    {
        LogEngineError("Max entity count reached, cannot reserve any more IDs");
        return nullptr;
    }

    m_LiveEntities[newID] = std::make_unique<Entity>(newID, name);
    return m_LiveEntities[newID].get();
}

void EntityManager::DestroyEntity(EntityID id)
{
    m_LiveEntities[id] = nullptr; // unique ptr should deallocate on its own
    ReleaseEntityID(id);

    // TODO: Should we update the scene graph? or should the world call this in the
    // first place?
}

EntityID EntityManager::ReserveNextEntityID()
{
    if (m_AvailableEntityIDs.empty())
        return ETH_INVALID_ENTITYID;

    EntityID nextID = m_AvailableEntityIDs.front();
    m_AvailableEntityIDs.pop();

    return nextID;
}

void EntityManager::ReleaseEntityID(EntityID id)
{
    m_AvailableEntityIDs.push(id);
}

ETH_NAMESPACE_END

