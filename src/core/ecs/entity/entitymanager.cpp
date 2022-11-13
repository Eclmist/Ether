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

#include "entitymanager.h"

ETH_NAMESPACE_BEGIN

EntityManager::EntityManager()
{
    for (EntityID id = 0; id < ETH_ECS_MAX_ENTITIES; ++id)
        m_AvailableEntityIDs.push(id);
}

EntityManager::~EntityManager()
{
}

Entity* EntityManager::CreateEntity(const std::string& name)
{
    EntityID newID = ReserveNextEntityID();

    if (newID == ETH_ECS_INVALID_ID)
    {
        LogEngineError("Max entity count reached, cannot reserve any more IDs");
        return nullptr;
    }

    m_LiveEntities[newID] = std::make_unique<Entity>(newID, name);
    m_LiveEntities[newID]->AddMandatoryComponents();
    m_GuidToEntityIDMap[m_LiveEntities[newID]->GetGuid()] = newID;
    EngineCore::GetActiveWorld().GetSceneGraph().Register(newID);
    return m_LiveEntities[newID].get();
}

void EntityManager::DestroyEntity(EntityID id)
{
    m_LiveEntities[id] = nullptr; // unique ptr should deallocate on its own
    ReleaseEntityID(id);
    EngineCore::GetActiveWorld().GetSceneGraph().Deregister(id);
}

EntityID EntityManager::ReserveNextEntityID()
{
    if (m_AvailableEntityIDs.empty())
        return ETH_ECS_INVALID_ID;

    EntityID nextID = m_AvailableEntityIDs.front();
    m_AvailableEntityIDs.pop();

    return nextID;
}

void EntityManager::ReleaseEntityID(EntityID id)
{
    m_AvailableEntityIDs.push(id);
}

#ifdef ETH_TOOLMODE

ethVector4u EntityManager::GetPickerColorFromID(EntityID id)
{
    ethVector4u color;
    uint32_t colorHash;

    if (m_EntityIDToColor.find(id) == m_EntityIDToColor.end())
    {
        do 
        {
            color = { (uint8_t)rand(), (uint8_t)rand(), (uint8_t)rand(), (uint8_t)rand() };
            colorHash = (uint32_t)(color.w);
            colorHash |= (uint32_t)(color.z << 8);
            colorHash |= (uint32_t)(color.w << 16);
            colorHash |= (uint32_t)(color.w << 24);
        } 
        while (m_ColorToEntityID.find(colorHash) != m_ColorToEntityID.end());
    }
    else
    {
        colorHash = m_EntityIDToColor.at(id);
        color.w = colorHash >> 0 & 0xFF;
        color.z = colorHash >> 8 & 0xFF;
        color.y = colorHash >> 16 & 0xFF;
        color.x = colorHash >> 24 & 0xFF;
    }

    return color;
}

EntityID EntityManager::GetIDFromPickerColor(ethVector4u color)
{
    uint32_t colorHash = 0;
    colorHash = (uint32_t)(color.w);
    colorHash |= (uint32_t)(color.z << 8);
    colorHash |= (uint32_t)(color.w << 16);
    colorHash |= (uint32_t)(color.w << 24);

    if (m_ColorToEntityID.find(colorHash) == m_ColorToEntityID.end())
        return ETH_ECS_INVALID_ID;

    return m_ColorToEntityID.at(colorHash);
}

#endif

ETH_NAMESPACE_END

