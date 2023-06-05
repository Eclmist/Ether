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

#include "engine/world/resources/resourcemanager.h"
#include "graphics/context/commandcontext.h"

constexpr uint32_t ResourceManagerVersion = 0;

Ether::ResourceManager::ResourceManager()
    : Serializable(ResourceManagerVersion, StringID("Engine::ResourceManager").GetHash())
{
}

void Ether::ResourceManager::Serialize(OStream& ostream) const
{
    Serializable::Serialize(ostream);
    SerializeResource<Graphics::Mesh>(ostream, m_Meshes);
    SerializeResource<Graphics::Material>(ostream, m_Materials);
}

void Ether::ResourceManager::Deserialize(IStream& istream)
{
    Serializable::Deserialize(istream);
    DeserializeResource<Graphics::Mesh>(istream, m_Meshes);
    DeserializeResource<Graphics::Material>(istream, m_Materials);
    CreateGpuResources();
}

Ether::StringID Ether::ResourceManager::RegisterMeshResource(std::unique_ptr<Graphics::Mesh>&& mesh)
{
    StringID sid = mesh->GetGuid();
    m_Meshes[sid] = std::move(mesh);
    return sid;
}

Ether::StringID Ether::ResourceManager::RegisterMaterialResource(std::unique_ptr<Graphics::Material>&& material)
{
    StringID sid = material->GetGuid();
    m_Materials[sid] = std::move(material);
    return sid;
}

Ether::Graphics::Mesh* Ether::ResourceManager::GetMeshResource(StringID id) const
{
    if (m_Meshes.find(id) == m_Meshes.end())
        return nullptr;

    return m_Meshes.at(id).get();
}

Ether::Graphics::Material* Ether::ResourceManager::GetMaterialResource(StringID id) const
{
    if (m_Materials.find(id) == m_Materials.end())
        return nullptr;

    return m_Materials.at(id).get();
}

void Ether::ResourceManager::CreateGpuResources() const 
{
    Graphics::CommandContext ctx(Graphics::RhiCommandType::Graphic, "CommandContext - Mesh Loading");

    ctx.Reset();
    for (auto& pair : m_Meshes)
    {
        std::string meshName = "Uploading mesh (" + pair.second->GetGuid() + ")";
        ctx.PushMarker(meshName);
        pair.second->CreateGpuResources(ctx);
        ctx.PopMarker();
    }
    ctx.FinalizeAndExecute(true);
}
