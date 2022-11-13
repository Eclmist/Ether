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

#include "componentarray.h"

ETH_NAMESPACE_BEGIN

class Component;

class ComponentManager : public NonCopyable // : public Serializable?
{
public:
    ComponentManager();
    ~ComponentManager() = default;

private:
    friend class EcsManager;
    
    template <typename T>
    std::shared_ptr<ComponentArray<T>> GetComponents() const
    {
        const char* typeName = typeid(T).name();
        return std::static_pointer_cast<ComponentArray<T>>(m_ComponentArrays.at(typeName));
    }

    template <typename T>
    T* GetComponent(EntityID id) const
    {
        return GetComponents<T>()->GetComponent(id);
    }

    template <typename T>
    T* AddComponent(EntityID id)
    {
        T* newComponent = GetComponents<T>()->AddComponent(id);
        m_GuidToComponentsMap[newComponent->GetGuid()] = newComponent;
        return newComponent;
    }

    template <typename T>
    void RemoveComponent(EntityID id)
    {
        T* oldComponent = GetComponents<T>()->GetComponent(id);
        m_GuidToComponentsMap.erase(oldComponent->GetGuid());
        GetComponents<T>()->RemoveComponent(id);
    }

    template <typename T>
    ComponentID GetComponentID() const
    {
        const char* typeName = typeid(T).name();
        return m_ComponentIDs.at(typeName);
    }

    template <typename T>
    void RegisterComponent()
    {
        const char* typeName = typeid(T).name();
        m_ComponentArrays.insert({ typeName, std::make_shared<ComponentArray<T>>() });
        m_ComponentIDs.insert({ typeName, m_NextComponentType++ });
    }

private:
    ETH_TOOLONLY(Component* GetComponentByGuid(const std::string& guid) const);

private:
    std::unordered_map<std::string, std::shared_ptr<IComponentArray>> m_ComponentArrays;
    std::unordered_map<std::string, ComponentID> m_ComponentIDs;

    std::unordered_map<std::string, Component*> m_GuidToComponentsMap;
    ComponentID m_NextComponentType;
};

ETH_NAMESPACE_END

