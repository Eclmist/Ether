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
#include "graphics/resources/mesh.h"
#include "graphics/resources/material.h"

namespace Ether
{
    class ResourceManager : public Serializable
    {
    public:
        ResourceManager();
        ~ResourceManager() = default;

        void Serialize(OStream& ostream) override;
        void Deserialize(IStream& istream) override;

    private:
        template <typename T>
        void SerializeResource(OStream& ostream, const std::unordered_map<StringID, std::unique_ptr<T>>& container);
        template <typename T>
        void DeserializeResource(IStream& istream, std::unordered_map<StringID, std::unique_ptr<T>>& container);

    private:
        friend class World;
        std::unordered_map<StringID, std::unique_ptr<Graphics::Mesh>> m_Mesh;
        std::unordered_map<StringID, std::unique_ptr<Graphics::Material>> m_Material;
        //std::unordered_map<StringID, Graphics::Texture> m_Texture;
    };

    template <typename T>
    void Ether::ResourceManager::SerializeResource(OStream& ostream, const std::unordered_map<StringID, std::unique_ptr<T>>& container)
    {
        ostream << (uint32_t)container.size();
        for (auto& pair : container)
            pair.second->Serialize(ostream);
    }

    template <typename T>
    void Ether::ResourceManager::DeserializeResource(IStream& istream, std::unordered_map<StringID, std::unique_ptr<T>>& container)
    {
        uint32_t numResources;
        istream >> numResources;
        for (int i = 0; i < numResources; ++i)
        {
            std::unique_ptr<T> resource = std::make_unique<T>();
            resource->Deserialize(istream);
            container.insert({ resource->GetGuid(), std::move(resource)});
        }
    }
}