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

#include "engine/pch.h"
#include "graphics/resources/mesh.h"
#include "graphics/resources/material.h"
#include "graphics/resources/texture.h"

namespace Ether
{
class ResourceManager : public Serializable
{
public:
    ResourceManager();
    ~ResourceManager() = default;

public:
    void Serialize(OStream& ostream) const override;
    void Deserialize(IStream& istream) override;

public:
    ETH_ENGINE_DLL StringID RegisterMeshResource(std::unique_ptr<Graphics::Mesh>&& mesh);
    ETH_ENGINE_DLL StringID RegisterMaterialResource(std::unique_ptr<Graphics::Material>&& material);
    ETH_ENGINE_DLL StringID RegisterTextureResource(std::unique_ptr<Graphics::Texture>&& texture);
    ETH_ENGINE_DLL void CreateGpuResources() const;

    Graphics::Mesh* GetMeshResource(StringID guid) const;
    Graphics::Material* GetMaterialResource(StringID guid) const;
    Graphics::Texture* GetTextureResource(StringID guid) const;

private:
    template <typename T>
    void SerializeResource(OStream& ostream, const std::unordered_map<StringID, std::unique_ptr<T>>& container) const;
    template <typename T>
    void DeserializeResource(IStream& istream, std::unordered_map<StringID, std::unique_ptr<T>>& container);

private:
    friend class World;
    std::unordered_map<StringID, std::unique_ptr<Graphics::Mesh>> m_Meshes;
    std::unordered_map<StringID, std::unique_ptr<Graphics::Material>> m_Materials;
    std::unordered_map<StringID, std::unique_ptr<Graphics::Texture>> m_Textures;
};

template <typename T>
void Ether::ResourceManager::SerializeResource(
    OStream& ostream,
    const std::unordered_map<StringID, std::unique_ptr<T>>& container) const
{
    ostream << static_cast<uint32_t>(container.size());
    for (auto& pair : container)
        pair.second->Serialize(ostream);
}

template <typename T>
void Ether::ResourceManager::DeserializeResource(
    IStream& istream,
    std::unordered_map<StringID, std::unique_ptr<T>>& container)
{
    uint32_t numResources;
    istream >> numResources;
    for (int i = 0; i < numResources; ++i)
    {
        std::unique_ptr<T> resource = std::make_unique<T>();
        resource->Deserialize(istream);
        container.insert({ resource->GetGuid(), std::move(resource) });
    }
}
} // namespace Ether