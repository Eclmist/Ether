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

#include "graphics/pch.h"
#include "graphics/common/vertexformats.h"

namespace Ether::Graphics
{
    class ETH_GRAPHIC_DLL Mesh : public Serializable
    {
    public:
        Mesh();
        ~Mesh() override = default;

    public:
        inline RhiVertexBufferViewDesc GetVertexBufferView() const { return m_VertexBufferView; }
        inline RhiIndexBufferViewDesc GetIndexBufferView() const { return m_IndexBufferView; }
        inline uint32_t GetNumVertices() const { return m_PackedVertices.size(); }
        inline uint32_t GetNumIndices() const { return m_Indices.size(); }

    public:
        void Serialize(OStream& ostream) override;
        void Deserialize(IStream& istream) override;

    public:
        void SetPackedVertices(std::vector<VertexFormats::PositionNormalTangentTexCoord>&& vertices);
        void SetIndices(std::vector<uint32_t>&& indices);
        void CreateGpuResources();

    private:
        void CreateVertexBuffer();
        void CreateVertexBufferView();
        void CreateIndexBufferView();
        void CreateIndexBuffer();
        void CreateInstanceParams();

    private:
        std::vector<VertexFormats::PositionNormalTangentTexCoord> m_PackedVertices;
        std::vector<uint32_t> m_Indices;

        std::unique_ptr<RhiResource> m_VertexBufferResource;
        std::unique_ptr<RhiResource> m_IndexBufferResource;
        std::unique_ptr<RhiResource> m_InstanceParams[MaxSwapChainBuffers];

        RhiVertexBufferViewDesc m_VertexBufferView;
        RhiIndexBufferViewDesc m_IndexBufferView;
    };
}

