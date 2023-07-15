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

#include "engine/world/world.h"
#include "engine/world/ecs/components/ecscameracomponent.h"

constexpr uint32_t WorldVersion = 0;

Ether::World::World()
    : Serializable(WorldVersion, "Engine::World")
    , m_WorldName("Default World")
    , m_MainCamera(nullptr)
{
}

void Ether::World::Update()
{
    ETH_MARKER_EVENT("World - Update");
    m_EcsManager.Update();
}

void Ether::World::Save(const std::string& path) const
{
    // TODO: Get target directory
    OFileStream outFile(path);
    outFile.ClearFile();
    Serialize(outFile);
}

void Ether::World::Load(const std::string& path)
{
    IFileStream inFile(path);
    Deserialize(inFile);
}

void Ether::World::Serialize(OStream& ostream) const
{
    Serializable::Serialize(ostream);
    ostream << m_WorldName;

    m_SceneGraph.Serialize(ostream);
    m_ResourceManager.Serialize(ostream);
    m_EcsManager.Serialize(ostream);

    ostream << static_cast<uint32_t>(m_Entities.size());
    for (auto& pair : m_Entities)
        pair.second->Serialize(ostream);
}

void Ether::World::Deserialize(IStream& istream)
{
    Serializable::Deserialize(istream);
    istream >> m_WorldName;

    m_SceneGraph.Deserialize(istream);
    m_ResourceManager.Deserialize(istream);
    m_EcsManager.Deserialize(istream);

    uint32_t numEntities;
    istream >> numEntities;

    for (int i = 0; i < numEntities; ++i)
    {
        std::unique_ptr<Entity> entity = std::make_unique<Entity>();
        entity->Deserialize(istream);
        auto sigToReregister = m_EcsManager.GetEntityManager().GetSignature(entity->GetID());
        m_EcsManager.GetSystemManager().UpdateEntitySignature(entity->GetID(), sigToReregister);
        m_Entities.insert({ entity->GetID(), std::move(entity) });
    }
}

Ether::Entity& Ether::World::CreateEntity(const std::string& name)
{
    Ecs::EntityID entityID = m_EcsManager.GetEntityManager().CreateEntity();
    std::unique_ptr<Entity> entity = std::make_unique<Entity>(name, entityID);
    m_Entities[entityID] = std::move(entity);
    return *m_Entities[entityID];
}

Ether::Entity& Ether::World::CreateCamera()
{
    if (m_MainCamera != nullptr)
        return *m_MainCamera;

    m_MainCamera = &CreateEntity("Main Camera");
    m_MainCamera->AddComponent<Ecs::EcsCameraComponent>();
    return *m_MainCamera;
}
