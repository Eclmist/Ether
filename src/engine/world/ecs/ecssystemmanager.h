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
#include "engine/world/ecs/systems/ecssystem.h"
#include "engine/world/ecs/ecstypes.h"
#include <typeindex>

namespace Ether::Ecs
{
    class EcsSystemManager : public NonCopyable, public NonMovable
    {
    public:
        EcsSystemManager() = default;
        ~EcsSystemManager() = default;

    public:
        template <typename T>
        void RegisterSystem(EcsSystem& system);

        ETH_ENGINE_DLL void UpdateEntitySignature(EntityID entityID, EntitySignature newSignature);
        ETH_ENGINE_DLL void OnEntityDestroyed(EntityID entityID);

    private:
        std::unordered_map<size_t, EcsSystem*> m_Systems;
    };

    template <typename T>
    void Ether::Ecs::EcsSystemManager::RegisterSystem(EcsSystem& system)
    {
        m_Systems[typeid(T).hash_code()] = &system;
    }
}

