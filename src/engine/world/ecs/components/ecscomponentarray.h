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

#pragma once

#include "pch.h"
#include "engine/world/ecs/ecstypes.h"
#include <array>

namespace Ether::Ecs
{
constexpr uint32_t ComponentArrayVersion = 0;

class EcsComponentArrayBase : public Serializable
{
public:
    EcsComponentArrayBase()
        : Serializable(ComponentArrayVersion, StringID("Engine::EcsComponentArray").GetHash())
    {
    }
    ~EcsComponentArrayBase() = default;

public:
    virtual void RemoveComponent(EntityID entityID) = 0;
};

template <typename T>
class EcsComponentArray : public EcsComponentArrayBase
{
public:
    EcsComponentArray() = default;
    ~EcsComponentArray() = default;

    void Serialize(OStream& ostream) const override
    {
        Serializable::Serialize(ostream);

        for (int i = 0; i < MaxNumEntities; ++i)
            m_ComponentArray[i].Serialize(ostream);

        ostream << m_NumElements;
        ostream << static_cast<uint32_t>(m_EntityToComponentIDMap.size());
        ostream << static_cast<uint32_t>(m_ComponentIDToEntityMap.size());

        for (auto& pair : m_EntityToComponentIDMap)
            ostream << pair.first << pair.second;

        for (auto& pair : m_ComponentIDToEntityMap)
            ostream << pair.first << pair.second;
    }

    void Deserialize(IStream& istream) override
    {
        Serializable::Deserialize(istream);

        for (int i = 0; i < MaxNumEntities; ++i)
            m_ComponentArray[i].Deserialize(istream);

        uint32_t compToIdMapSize, entityToCompMapSize;
        istream >> m_NumElements;
        istream >> entityToCompMapSize;
        istream >> compToIdMapSize;

        uint32_t first, second;
        for (int i = 0; i < entityToCompMapSize; ++i)
        {
            istream >> first;
            istream >> second;
            m_EntityToComponentIDMap[first] = second;
        }

        for (int i = 0; i < compToIdMapSize; ++i)
        {
            istream >> first;
            istream >> second;
            m_ComponentIDToEntityMap[first] = second;
        }
    }

public:
    inline T& GetComponent(EntityID entityID) { return m_ComponentArray[m_EntityToComponentIDMap[entityID]]; }

public:
    void AddComponent(EntityID entityID);
    void RemoveComponent(EntityID entityID) override;

private:
    std::array<T, MaxNumEntities> m_ComponentArray;
    std::unordered_map<EntityID, uint32_t> m_EntityToComponentIDMap;
    std::unordered_map<uint32_t, EntityID> m_ComponentIDToEntityMap;

    uint32_t m_NumElements;
};

template <typename T>
void Ether::Ecs::EcsComponentArray<T>::AddComponent(EntityID entityID)
{
    if (m_NumElements >= MaxNumEntities)
        throw std::runtime_error("Component array is full");

    uint32_t newIdx = m_NumElements;
    m_EntityToComponentIDMap[entityID] = newIdx;
    m_ComponentIDToEntityMap[newIdx] = entityID;
    m_ComponentArray[newIdx] = {};

    m_NumElements++;
}

template <typename T>
void Ether::Ecs::EcsComponentArray<T>::RemoveComponent(EntityID entityID)
{
    if (m_EntityToComponentIDMap.find(entityID) == m_EntityToComponentIDMap.end())
        return;

    size_t currIdx = m_EntityToComponentIDMap[entityID];
    size_t lastIdx = m_NumElements - 1;
    m_ComponentArray[m_EntityToComponentIDMap[entityID]] = m_ComponentArray[lastIdx];

    EntityID lastEntityID = m_ComponentIDToEntityMap[lastIdx];
    m_EntityToComponentIDMap[lastEntityID] = currIdx;
    m_ComponentIDToEntityMap[currIdx] = lastEntityID;

    m_EntityToComponentIDMap.erase(entityID);
    m_ComponentIDToEntityMap.erase(lastIdx);

    m_NumElements--;
}
} // namespace Ether::Ecs
