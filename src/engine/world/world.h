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
#include "engine/world/resources/resourcemanager.h"

namespace Ether
{
class ETH_ENGINE_DLL World : public Serializable
{
public:
    World();
    ~World() override = default;

public:
    void Update();
    void Save(const std::string& path) const;
    void Load(const std::string& path);
    void Unload();

public:
    inline std::string GetWorldName() const { return m_WorldName; }
    inline Entity& GetEntity(Ecs::EntityID entityID) const { return *m_Entities.at(entityID); }
    inline SceneGraph& GetSceneGraph() { return m_SceneGraph; }
    inline ResourceManager& GetResourceManager() { return m_ResourceManager; }
    inline Ecs::EcsManager& GetEcsManager() { return m_EcsManager; }
    inline Entity* GetMainCamera() { return m_MainCamera; }

    inline void SetWorldName(const std::string& name) { m_WorldName = name; }

public:
    Entity& CreateEntity(const std::string& name);
    Entity& CreateCamera();

private:
    void Serialize(OStream& ostream) const override;
    void Deserialize(IStream& istream) override;

private:
    std::string m_WorldName;

    SceneGraph m_SceneGraph;
    ResourceManager m_ResourceManager;

    Ecs::EcsManager m_EcsManager;
    std::unordered_map<Ecs::EntityID, std::unique_ptr<Entity>> m_Entities;

    Entity* m_MainCamera;
};

} // namespace Ether
