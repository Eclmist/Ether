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
#include "engine/world/ecs/component/ecscomponentarray.h"
#include <typeindex>

namespace Ether::Ecs
{
    class EcsComponentManager : public NonCopyable, public NonMovable
    {
    public:
        EcsComponentManager() = default;
        ~EcsComponentManager() = default;

    public:
        template <typename T>
        void RegisterComponent()
        {
            if (m_ComponentArrays.find(typeid(T)) != m_ComponentArrays.end())
                throw std::logic_error("Same component type is registered more than once");

            m_ComponentArrays[typeid(T)] = std::make_unique<EcsComponentArray<T>>();
        }

        template <typename T>
        T& GetComponent(EntityID entityID)
        {
            return dynamic_cast<EcsComponentArray<T>&>(*m_ComponentArrays.at(typeid(T))).GetComponent(entityID);
        }

        template <typename T>
        void AddComponent(EntityID entityID)
        {
            dynamic_cast<EcsComponentArray<T>&>(*m_ComponentArrays.at(typeid(T))).AddComponent(entityID);
        }

        template <typename T>
        void RemoveComponent(EntityID entityID)
        {
            dynamic_cast<EcsComponentArray<T>&>(*m_ComponentArrays.at(typeid(T))).RemoveComponent(entityID);
        }

        void OnEntityDestroyed(EntityID entityID)
        {
            for (auto const& pair : m_ComponentArrays)
                pair.second->RemoveComponent(entityID);
        }

    private:
        std::unordered_map<std::type_index, std::unique_ptr<EcsComponentArrayBase>> m_ComponentArrays;
    };
}
