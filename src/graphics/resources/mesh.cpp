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

#include "graphics/resources/mesh.h"
#include "graphics/graphiccore.h"

constexpr uint32_t MeshVersion = 3;

Ether::Graphics::Mesh::Mesh()
    : Serializable(MeshVersion, ETH_CLASS_ID_MESH)
    , m_IndexBufferView({})
    , m_VertexBufferView({})
{
}

void Ether::Graphics::Mesh::Serialize(OStream& ostream) const
{
    Serializable::Serialize(ostream);

    ostream << static_cast<uint32_t>(m_PackedVertices.size());
    for (int i = 0; i < m_PackedVertices.size(); ++i)
        m_PackedVertices[i].Serialize(ostream);

    ostream << static_cast<uint32_t>(m_Indices.size());
    for (int i = 0; i < m_Indices.size(); ++i)
        ostream << m_Indices[i];

    ostream << m_DefaultMaterialGuid.GetString();
}

void Ether::Graphics::Mesh::Deserialize(IStream& istream)
{
    Serializable::Deserialize(istream);

    uint32_t numVertices = 0;
    istream >> numVertices;

    m_PackedVertices.resize(numVertices);
    for (int i = 0; i < m_PackedVertices.size(); ++i)
        m_PackedVertices[i].Deserialize(istream);

    uint32_t numIndices = 0;
    istream >> numIndices;

    m_Indices.resize(numIndices);
    for (int i = 0; i < m_Indices.size(); ++i)
        istream >> m_Indices[i];

    istream >> m_DefaultMaterialGuid;
}

void Ether::Graphics::Mesh::SetPackedVertices(
    std::vector<VertexFormats::PositionNormalTangentBitangentTexcoord>&& vertices)
{
    m_PackedVertices = std::move(vertices);
}

void Ether::Graphics::Mesh::SetIndices(std::vector<uint32_t>&& indices)
{
    m_Indices = indices;
}

void Ether::Graphics::Mesh::CreateGpuResources(CommandContext& ctx)
{
    CreateVertexBuffer(ctx);
    CreateIndexBuffer(ctx);
    CreateAccelerationStructure(ctx);
}

void Ether::Graphics::Mesh::CreateVertexBuffer(CommandContext& ctx)
{
    size_t bufferSize = m_PackedVertices.size() * sizeof(m_PackedVertices[0]);
    RhiCommitedResourceDesc desc = {};
    desc.m_Name = "Mesh::VertexBuffer";
    desc.m_HeapType = RhiHeapType::Default;
    desc.m_State = RhiResourceState::Common;
    desc.m_ResourceDesc = RhiCreateBufferResourceDesc(bufferSize);

    m_VertexBufferResource = GraphicCore::GetDevice().CreateCommittedResource(desc);
    ctx.PushMarker("Vertex Buffer Upload");
    ctx.InitializeBufferRegion(*m_VertexBufferResource, m_PackedVertices.data(), bufferSize);
    ctx.PopMarker();

    CreateVertexBufferView();
}

void Ether::Graphics::Mesh::CreateIndexBuffer(CommandContext& ctx)
{
    size_t bufferSize = m_Indices.size() * sizeof(m_Indices[0]);

    RhiCommitedResourceDesc desc = {};
    desc.m_Name = "Mesh::IndexBuffer";
    desc.m_HeapType = RhiHeapType::Default;
    desc.m_State = RhiResourceState::Common;
    desc.m_ResourceDesc = RhiCreateBufferResourceDesc(bufferSize);

    m_IndexBufferResource = GraphicCore::GetDevice().CreateCommittedResource(desc);
    ctx.PushMarker("Index Buffer Upload");
    ctx.InitializeBufferRegion(*m_IndexBufferResource, m_Indices.data(), bufferSize);
    ctx.PopMarker();

    CreateIndexBufferView();
}

void Ether::Graphics::Mesh::CreateAccelerationStructure(CommandContext& ctx)
{
    RhiBottomLevelAccelerationStructureDesc desc = {};
    Mesh* meshes[] = { this };
    desc.m_Meshes = (void**)meshes;
    desc.m_NumMeshes = 1;
    desc.m_IsOpaque = true;
    desc.m_IsStatic = true;

    m_AccelerationStructure = GraphicCore::GetDevice().CreateAccelerationStructure(desc);
    ctx.PushMarker("Build BLAS");
    ctx.TransitionResource(*m_AccelerationStructure->m_ScratchBuffer, RhiResourceState::UnorderedAccess);
    ctx.BuildBottomLevelAccelerationStructure(*m_AccelerationStructure);
    ctx.PopMarker();
}

void Ether::Graphics::Mesh::CreateVertexBufferView()
{
    m_VertexBufferView = {};
    m_VertexBufferView.m_BufferSize = m_PackedVertices.size() * sizeof(m_PackedVertices[0]);
    m_VertexBufferView.m_Stride = sizeof(m_PackedVertices[0]);
    m_VertexBufferView.m_TargetGpuAddress = m_VertexBufferResource->GetGpuAddress();
}

void Ether::Graphics::Mesh::CreateIndexBufferView()
{
    m_IndexBufferView = {};
    m_IndexBufferView.m_BufferSize = m_Indices.size() * sizeof(m_Indices[0]);
    m_IndexBufferView.m_Format = s_IndexBufferFormat;
    m_IndexBufferView.m_TargetGpuAddress = m_IndexBufferResource->GetGpuAddress();
}
