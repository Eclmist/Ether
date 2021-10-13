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

#include "componentarray.h"

ETH_NAMESPACE_BEGIN

class Component;

class ComponentManager : public NonCopyable // : public Serializable?
{
public:
    ComponentManager();
    ~ComponentManager() = default;

private:
    friend class ECSManager;
    
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
        return GetComponents<T>()->AddComponent(id);
    }

    template <typename T>
    void RemoveComponent(EntityID id)
    {
        GetComponents<T>()->RemoveComponent(id);
    }

    template <typename T>
    ComponentID GetComponentID() const
    {
        const char* typeName = typeid(T).name();
        return m_ComponentIDs.at(typeName);
    }

private:
    template <typename T>
    void RegisterComponent()
    {
        const char* typeName = typeid(T).name();
        m_ComponentArrays.insert({ typeName, std::make_shared<ComponentArray<T>>() });
        m_ComponentIDs.insert({ typeName, m_NextComponentType++ });
    }

private:
    std::unordered_map<std::string, std::shared_ptr<IComponentArray>> m_ComponentArrays;
    std::unordered_map<std::string, ComponentID> m_ComponentIDs;
    ComponentID m_NextComponentType;
};

ETH_NAMESPACE_END

