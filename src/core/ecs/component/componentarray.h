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

ETH_NAMESPACE_BEGIN

class IComponentArray
{
public:
    virtual ~IComponentArray() = default;
};

template <typename T>
class ComponentArray : public IComponentArray
{
public:
    ComponentArray() = default;
    ~ComponentArray() = default;

public:
    T* GetComponent(EntityID id); 
    T* AddComponent(EntityID id);
    void RemoveComponent(EntityID id);

private:
    std::unordered_map<EntityID, uint32_t> m_EntityToIndexMap;
    std::unordered_map<uint32_t, EntityID> m_IndexToEntityMap;
    std::vector<std::unique_ptr<T>> m_Components;
};

template <typename T>
T* ComponentArray<T>::GetComponent(EntityID id)
{
    if (m_EntityToIndexMap.find(id) == m_EntityToIndexMap.end())
        return nullptr;

    return m_Components[m_EntityToIndexMap[id]].get();
}

template <typename T>
T* ComponentArray<T>::AddComponent(EntityID id)
{
    m_Components.emplace_back(std::make_unique<T>(id));
    uint32_t index = m_Components.size() - 1;

    m_EntityToIndexMap[id] = index;
    m_IndexToEntityMap[index] = id;
    return m_Components.back().get();
}

template <typename T>
void ComponentArray<T>::RemoveComponent(EntityID id)
{
    uint32_t removalIndex = m_EntityToIndexMap[id];
    uint32_t lastElementIndex = m_Components.size() - 1;

    if (removalIndex != lastElementIndex);
    {
        // Copy last component to fill the gap
        m_Components[removalIndex] = m_Components[lastElementIndex];

        // Update index maps
        EntityID movedElementID = m_IndexToEntityMap[lastElementIndex];
        m_IndexToEntityMap[removalIndex] = movedElementID;
        m_EntityToIndexMap[movedElementID] = removalIndex;
        m_IndexToEntityMap.erase(lastElementIndex);
        m_EntityToIndexMap.erase(id);
    }

    m_Components.pop_back();
}

ETH_NAMESPACE_END

