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
#include "graphics/context/commandcontext.h"
#include "graphics/rhi/rhiaccelerationstructure.h"

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
    inline RhiAccelerationStructure& GetAccelerationStructure() const { return *m_AccelerationStructure; }
    inline uint32_t GetNumVertices() const { return m_PackedVertices.size(); }
    inline uint32_t GetNumIndices() const { return m_Indices.size(); }

public:
    void Serialize(OStream& ostream) const override;
    void Deserialize(IStream& istream) override;

public:
    void SetPackedVertices(std::vector<VertexFormats::PositionNormalTangentBitangentTexcoord>&& vertices);
    void SetIndices(std::vector<uint32_t>&& indices);
    void CreateGpuResources(CommandContext& ctx);

public:
    static constexpr RhiFormat s_VertexBufferPositionFormat = RhiFormat::R32G32B32Float;
    static constexpr RhiFormat s_IndexBufferFormat = RhiFormat::R32Uint;

private:
    void CreateVertexBuffer(CommandContext& ctx);
    void CreateIndexBuffer(CommandContext& ctx);
    void CreateAccelerationStructure(CommandContext& ctx);

    void CreateVertexBufferView();
    void CreateIndexBufferView();

private:
    std::vector<VertexFormats::PositionNormalTangentBitangentTexcoord> m_PackedVertices;
    std::vector<uint32_t> m_Indices;

    std::unique_ptr<RhiResource> m_VertexBufferResource;
    std::unique_ptr<RhiResource> m_IndexBufferResource;
    std::unique_ptr<RhiResource> m_InstanceParams[MaxSwapChainBuffers];
    std::unique_ptr<RhiAccelerationStructure> m_AccelerationStructure;

    RhiVertexBufferViewDesc m_VertexBufferView;
    RhiIndexBufferViewDesc m_IndexBufferView;
};
} // namespace Ether::Graphics
