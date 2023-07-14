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

#define ETH_CLASS_ID_MESH "Graphics::Mesh"

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
    inline uint32_t GetVertexBufferSrvIndex() const { return m_VertexBufferSrvIndex; }
    inline uint32_t GetIndexBufferSrvIndex() const { return m_IndexBufferSrvIndex; }
    inline RhiAccelerationStructure& GetAccelerationStructure() const { return *m_AccelerationStructure; }
    inline uint32_t GetNumVertices() const { return m_NumVertices; }
    inline uint32_t GetNumIndices() const { return m_NumIndices; }
    inline StringID GetDefaultMaterialGuid() const { return m_DefaultMaterialGuid; }

public:
    void Serialize(OStream& ostream) const override;
    void Deserialize(IStream& istream) override;

public:
    void SetDefaultMaterialGuid(StringID guid) { m_DefaultMaterialGuid = guid; }
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

    void InitializeVertexBufferViews();
    void InitializeIndexBufferViews();

private:
    std::vector<VertexFormats::PositionNormalTangentBitangentTexcoord> m_PackedVertices;
    std::vector<uint32_t> m_Indices;
    uint32_t m_NumVertices;
    uint32_t m_NumIndices;
    StringID m_DefaultMaterialGuid;

    // Transient Data
    std::unique_ptr<RhiResource> m_VertexBufferResource;
    std::unique_ptr<RhiResource> m_IndexBufferResource;
    std::unique_ptr<RhiResource> m_InstanceParams[MaxSwapChainBuffers];
    std::unique_ptr<RhiAccelerationStructure> m_AccelerationStructure;

    RhiVertexBufferViewDesc m_VertexBufferView;
    RhiIndexBufferViewDesc m_IndexBufferView;
    uint32_t m_VertexBufferSrvIndex;
    uint32_t m_IndexBufferSrvIndex;

    std::string m_VbName;
    std::string m_IbName;
};
} // namespace Ether::Graphics
