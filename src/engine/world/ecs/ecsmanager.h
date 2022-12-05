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
#include "engine/world/ecs/ecsentitymanager.h"
#include "engine/world/ecs/ecscomponentmanager.h"
#include "engine/world/ecs/ecssystemmanager.h"

namespace Ether::Ecs
{
    class ETH_ENGINE_DLL EcsManager : public Serializable
    {
    public:
        EcsManager();
        ~EcsManager() = default;

    public:
        void Serialize(OStream& ostream) const override;
        void Deserialize(IStream& istream) override;

    public:
        void Update();

    public:
        inline EcsEntityManager& GetEntityManager() { return m_EntityManager; }
        inline EcsComponentManager& GetComponentManager() { return m_ComponentManager; }
        inline EcsSystemManager& GetSystemManager() { return m_SystemManager; }

    public:
        EntityID CreateEntity();
        void DestroyEntity(EntityID entityID);

    private:
        EcsEntityManager m_EntityManager;
        EcsComponentManager m_ComponentManager;
        EcsSystemManager m_SystemManager;
    };
}

