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

constexpr uint32_t MeshVersion = 7;

Ether::Graphics::Mesh::Mesh()
    : Serializable(MeshVersion, ETH_CLASS_ID_MESH)
    , m_NumVertices(0)
    , m_NumIndices(0)
    , m_IndexBufferView({})
    , m_VertexBufferView({})
{
}

void Ether::Graphics::Mesh::Serialize(OStream& ostream) const
{
    Serializable::Serialize(ostream);

    ostream << m_NumVertices;
    for (int i = 0; i < m_PackedVertices.size(); ++i)
        m_PackedVertices[i].Serialize(ostream);

    ostream << m_NumIndices;
    for (int i = 0; i < m_Indices.size(); ++i)
        ostream << m_Indices[i];

    ostream << m_DefaultMaterialGuid.GetString();
    ostream << m_BoundingBox.m_Min;
    ostream << m_BoundingBox.m_Max;
}

void Ether::Graphics::Mesh::Deserialize(IStream& istream)
{
    Serializable::Deserialize(istream);

    istream >> m_NumVertices;
    AssertGraphics(m_NumVertices <= MaxVerticesPerMesh, "Num vertices exceeds limit");

    m_PackedVertices.resize(m_NumVertices);
    for (int i = 0; i < m_PackedVertices.size(); ++i)
        m_PackedVertices[i].Deserialize(istream);

    istream >> m_NumIndices;
    AssertGraphics(m_NumIndices <= MaxTrianglePerMesh * 3, "Num triangles exceeds limit");

    m_Indices.resize(m_NumIndices);
    for (int i = 0; i < m_Indices.size(); ++i)
        istream >> m_Indices[i];

    istream >> m_DefaultMaterialGuid;
    istream >> (ethVector3&)m_BoundingBox.m_Min;
    istream >> (ethVector3&)m_BoundingBox.m_Max;
}

void Ether::Graphics::Mesh::SetPackedVertices(
    std::vector<VertexFormats::PositionNormalTangentTexcoord>&& vertices)
{
    m_PackedVertices = std::move(vertices);
    m_NumVertices = m_PackedVertices.size();

    m_BoundingBox.m_Min = 9999999;
    m_BoundingBox.m_Max = -9999999;

    for (auto& vertex : m_PackedVertices)
    {
        m_BoundingBox.m_Min.x = std::min(m_BoundingBox.m_Min.x, vertex.m_Position.x);
        m_BoundingBox.m_Min.y = std::min(m_BoundingBox.m_Min.y, vertex.m_Position.y);
        m_BoundingBox.m_Min.z = std::min(m_BoundingBox.m_Min.z, vertex.m_Position.z);
    
        m_BoundingBox.m_Max.x = std::max(m_BoundingBox.m_Max.x, vertex.m_Position.x);
        m_BoundingBox.m_Max.y = std::max(m_BoundingBox.m_Max.y, vertex.m_Position.y);
        m_BoundingBox.m_Max.z = std::max(m_BoundingBox.m_Max.z, vertex.m_Position.z);
    }
}

void Ether::Graphics::Mesh::SetIndices(std::vector<uint32_t>&& indices)
{
    m_Indices = indices;
    m_NumIndices = m_Indices.size();
}

void Ether::Graphics::Mesh::CreateGpuResources(CommandContext& ctx)
{
    CreateVertexBuffer(ctx);
    CreateIndexBuffer(ctx);
    CreateAccelerationStructure(ctx);

#ifdef ETH_ENGINE
    // Mesh data can be deallocated on the CPU. It's all in VRAM now.
    // This might cause problems down the line, but if it is not deallocated the CPU memory usage is going to be crazy
    m_PackedVertices.clear();
    m_Indices.clear();
#endif
}

void Ether::Graphics::Mesh::CreateVertexBuffer(CommandContext& ctx)
{
    m_VbName = "Mesh::VertexBuffer (" + GetGuid() + ")";
    size_t bufferSize = m_PackedVertices.size() * sizeof(m_PackedVertices[0]);
    RhiCommitedResourceDesc desc = {};
    desc.m_Name = m_VbName.c_str();
    desc.m_HeapType = RhiHeapType::Default;
    desc.m_State = RhiResourceState::Common;
    desc.m_ResourceDesc = RhiCreateBufferResourceDesc(bufferSize);

    m_VertexBufferResource = GraphicCore::GetDevice().CreateCommittedResource(desc);
    ctx.PushMarker("Vertex Buffer Upload");
    ctx.InitializeBufferRegion(*m_VertexBufferResource, m_PackedVertices.data(), bufferSize);
    ctx.PopMarker();

    InitializeVertexBufferViews();
}

void Ether::Graphics::Mesh::CreateIndexBuffer(CommandContext& ctx)
{
    m_IbName = "Mesh::IndexBuffer (" + GetGuid() + ")";
    size_t bufferSize = m_Indices.size() * sizeof(m_Indices[0]);

    RhiCommitedResourceDesc desc = {};
    desc.m_Name = m_IbName.c_str();
    desc.m_HeapType = RhiHeapType::Default;
    desc.m_State = RhiResourceState::Common;
    desc.m_ResourceDesc = RhiCreateBufferResourceDesc(bufferSize);

    m_IndexBufferResource = GraphicCore::GetDevice().CreateCommittedResource(desc);
    ctx.PushMarker("Index Buffer Upload");
    ctx.InitializeBufferRegion(*m_IndexBufferResource, m_Indices.data(), bufferSize);
    ctx.PopMarker();

    InitializeIndexBufferViews();
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

void Ether::Graphics::Mesh::InitializeVertexBufferViews()
{
    m_VertexBufferView = {};
    m_VertexBufferView.m_BufferSize = m_PackedVertices.size() * sizeof(m_PackedVertices[0]);
    m_VertexBufferView.m_Stride = sizeof(m_PackedVertices[0]);
    m_VertexBufferView.m_TargetGpuAddress = m_VertexBufferResource->GetGpuAddress();

    m_VertexBufferSrvIndex = GraphicCore::GetBindlessDescriptorManager().RegisterAsShaderResourceView(
        m_VbName + " SRV",
        *m_VertexBufferResource,
        m_VertexBufferView);
}

void Ether::Graphics::Mesh::InitializeIndexBufferViews()
{
    m_IndexBufferView = {};
    m_IndexBufferView.m_BufferSize = m_Indices.size() * sizeof(m_Indices[0]);
    m_IndexBufferView.m_Format = s_IndexBufferFormat;
    m_IndexBufferView.m_TargetGpuAddress = m_IndexBufferResource->GetGpuAddress();

    m_IndexBufferSrvIndex = GraphicCore::GetBindlessDescriptorManager().RegisterAsShaderResourceView(
        m_IbName + " SRV",
        *m_IndexBufferResource,
        m_IndexBufferView);
}
