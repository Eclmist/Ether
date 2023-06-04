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
#include "engine/world/ecs/components/ecscomponentarray.h"

namespace Ether::Ecs
{
class EcsComponentManager : public Serializable
{
public:
    EcsComponentManager();
    ~EcsComponentManager() = default;

public:
    void Serialize(OStream& ostream) const override;
    void Deserialize(IStream& istream) override;

    template <typename T>
    ComponentID GetTypeID()
    {
        return m_TypeNameToIDMap[typeid(T).name()];
    }

    template <typename T>
    T& GetComponent(EntityID entityID)
    {
        return dynamic_cast<EcsComponentArray<T>&>(*m_ComponentArrays.at(GetTypeID<T>())).GetComponent(entityID);
    }

    template <typename T>
    void AddComponent(EntityID entityID)
    {
        dynamic_cast<EcsComponentArray<T>&>(*m_ComponentArrays.at(GetTypeID<T>())).AddComponent(entityID);
    }

    template <typename T>
    void RemoveComponent(EntityID entityID)
    {
        dynamic_cast<EcsComponentArray<T>&>(*m_ComponentArrays.at(GetTypeID<T>())).RemoveComponent(entityID);
    }

    void OnEntityDestroyed(EntityID entityID)
    {
        for (auto const& pair : m_ComponentArrays)
            pair.second->RemoveComponent(entityID);
    }

private:
    template <typename T>
    void RegisterComponent()
    {
        if (m_TypeNameToIDMap.find(typeid(T).name()) != m_TypeNameToIDMap.end())
            LogEngineError("Same component type is registered more than once");

        ComponentID newID = m_NextID++;
        m_TypeNameToIDMap[typeid(T).name()] = newID;
        T::s_ComponentID = newID;
        m_ComponentArrays[newID] = std::make_unique<EcsComponentArray<T>>();
    }

private:
    std::unordered_map<std::string, ComponentID> m_TypeNameToIDMap;
    std::unordered_map<ComponentID, std::unique_ptr<EcsComponentArrayBase>> m_ComponentArrays;

    ComponentID m_NextID;
};
} // namespace Ether::Ecs
