/*
    This file is part of Ether, an open-source DirectX 12 renderer.

    Copyright (c) 2020-2030 Samuel Huang - All rights reserved.

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

#include "engine/pch.h"
#include "engine/world/ecs/ecsmanager.h"
#include "engine/world/scenegraph.h"

namespace Ether
{
class ETH_ENGINE_DLL Entity : public Serializable
{
public:
    Entity(); // For use during deserialization
    Entity(const std::string& name, Ecs::EntityID entityID);
    ~Entity() = default;

public:
    void Serialize(OStream& ostream) const override;
    void Deserialize(IStream& istream) override;

public:
    inline Ecs::EntityID GetID() const { return m_EntityID; }

public:
    std::string GetName();
    bool IsEnabled();

public:
    bool HasComponent(Ecs::ComponentID id);
    bool AddComponent(Ecs::ComponentID id);
    bool RemoveComponent(Ecs::ComponentID id);

public:
    template <typename T>
    bool HasComponent();
    template <typename T>
    T& GetComponent();
    template <typename T>
    T& AddComponent();
    template <typename T>
    void RemoveComponent();

private:
    Ecs::EntityID m_EntityID;

    Ecs::EcsSystemManager& m_SystemsManager;
    Ecs::EcsEntityManager& m_EntityManager;
    Ecs::EcsComponentManager& m_ComponentManager;

    SceneGraph& m_SceneGraph;
};

template <typename T>
bool Ether::Entity::HasComponent()
{
    return m_EntityManager.GetSignature(GetID()).test(m_ComponentManager.GetTypeID<T>());
}

template <typename T>
T& Ether::Entity::GetComponent()
{
    return m_ComponentManager.GetComponent<T>(GetID());
}

template <typename T>
T& Ether::Entity::AddComponent()
{
    auto signature = m_EntityManager.GetSignature(GetID());

    // Component already exist. Enforce only one component of each type for entity.
    if (signature.test(m_ComponentManager.GetTypeID<T>()))
        return GetComponent<T>();

    m_ComponentManager.AddComponent<T>(GetID());

    // Update signature
    signature.set(m_ComponentManager.GetTypeID<T>());
    m_EntityManager.SetSignature(GetID(), signature);
    m_SystemsManager.UpdateEntitySignature(GetID(), signature);

    return GetComponent<T>();
}

template <typename T>
void Ether::Entity::RemoveComponent()
{
    m_ComponentManager.RemoveComponent<T>();
}
} // namespace Ether
