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
#include "engine/world/ecs/ecsmanager.h"
#include "engine/world/scenegraph.h"

namespace Ether
{
    namespace Ecs
    {
        class EcsEntityDataComponent;
        class EcsTransformComponent;
    }

    class ETH_ENGINE_DLL Entity : public NonCopyable, public NonMovable
    {
    public:
        Entity(const std::string& name);
        ~Entity() = default;

    public:
        inline Ecs::EntityID GetID() const { return m_EntityID; }

    public:
        std::string GetName();
        bool IsEnabled();

    public:
        template <typename T>
        T& GetComponent();

        template <typename T>
        void AddComponent();

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
    T& Ether::Entity::GetComponent()
    {
        return m_ComponentManager.GetComponent<T>(GetID());
    }

    template <typename T>
    void Ether::Entity::AddComponent()
    {
        m_ComponentManager.AddComponent<T>(GetID());
        
        // Update signature
        auto signature = m_EntityManager.GetSignature(GetID());

        signature.set(T::s_ComponentID);
        m_EntityManager.SetSignature(GetID(), signature);
        m_SystemsManager.UpdateEntitySignature(GetID(), signature);
    }

    template <typename T>
    void Ether::Entity::RemoveComponent()
    {
        m_ComponentManager.RemoveComponent<T>();
    }
}

