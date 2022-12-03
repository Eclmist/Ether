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
#include "engine/world/entity.h"
#include "engine/world/scenegraph.h"
#include "engine/world/ecs/ecsmanager.h"

namespace Ether
{
    class ETH_ENGINE_DLL World : public Serializable
    {
    public:
        World();
        ~World() override = default;

        void Update();

    public:
        void Serialize(OStream& ostream) override;
        void Deserialize(IStream& istream) override;

    public:
        inline std::string GetWorldName() const { return m_WorldName; }

    public:
        Entity& CreateEntity(const std::string& name);

    private:
        friend class Entity;
        inline SceneGraph& GetSceneGraph() { return m_SceneGraph; }
        inline Ecs::EcsManager& GetEcsManager() { return m_EcsManager; }

    private:
        std::string m_WorldName;

        SceneGraph m_SceneGraph;
        Ecs::EcsManager m_EcsManager;

        std::unordered_map<Ecs::EntityID, std::unique_ptr<Entity>> m_Entities;
    };
}
