/*
    This file is part of Ether, an open-source DirectX 12 renderer.

    Copyright (c) 2020-2030 Samuel Huang - All rights reserved.

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

Ether::Graphics::Mesh::Mesh(uint32_t version, const char* classID)
    : Serializable(version, classID)
    , m_NumVertices(0)
    , m_NumIndices(0)
    , m_IndexBufferView({})
    , m_VertexBufferView({})
{
}

void Ether::Graphics::Mesh::Serialize(OStream& ostream) const
{
    Serializable::Serialize(ostream);

    ostream << m_NumIndices;
    for (int i = 0; i < m_Indices.size(); ++i)
        ostream << m_Indices[i];

    ostream << m_DefaultMaterialGuid.GetString();
    ostream << m_BoundingBox.m_Min;
    ostream << m_BoundingBox.m_Max;

    ostream << m_AssetTransform;
}

void Ether::Graphics::Mesh::Deserialize(IStream& istream)
{
    Serializable::Deserialize(istream);

    istream >> m_NumIndices;
    m_Indices.resize(m_NumIndices);
    for (int i = 0; i < m_Indices.size(); ++i)
        istream >> m_Indices[i];

    istream >> m_DefaultMaterialGuid;
    istream >> (ethVector3&)m_BoundingBox.m_Min;
    istream >> (ethVector3&)m_BoundingBox.m_Max;

    istream >> m_AssetTransform;
}

void Ether::Graphics::Mesh::SetIndices(std::vector<uint32_t>&& indices)
{
    m_Indices = indices;
    m_NumIndices = m_Indices.size();
}

void Ether::Graphics::Mesh::CreateVertexBuffer(CommandContext& ctx)
{
    m_VbName = "Mesh::VertexBuffer (" + GetGuid() + ")";
    size_t bufferSize = m_NumVertices * GetVertexStride();
    RhiCommitedResourceDesc desc = {};
    desc.m_Name = m_VbName.c_str();
    desc.m_HeapType = RhiHeapType::Default;
    desc.m_State = RhiResourceState::Common;
    desc.m_ResourceDesc = RhiCreateBufferResourceDesc(bufferSize);

    m_VertexBufferResource = GraphicCore::GetDevice().CreateCommittedResource(desc);
    ctx.PushMarker("Vertex Buffer Upload");
    ctx.InitializeBufferRegion(*m_VertexBufferResource, GetPackedVertexData(), bufferSize);
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

void Ether::Graphics::Mesh::CreateAccelerationStructure(CommandContext& ctx, bool allowUpdate)
{
    RhiBottomLevelAccelerationStructureDesc desc = {};

    // Don't generate BLAS for degenerate meshes
    if (GetNumVertices() == 0 || GetNumIndices() < 3)
        return;

    Mesh* meshes[] = { this };
    desc.m_Meshes = (void**)meshes;
    desc.m_NumMeshes = 1;
    desc.m_IsOpaque = false;
    desc.m_IsStatic = !allowUpdate;

    m_AccelerationStructure = GraphicCore::GetDevice().CreateAccelerationStructure(desc);
    ctx.PushMarker("Build BLAS");
    ctx.TransitionResource(*m_AccelerationStructure->m_ScratchBuffer, RhiResourceState::UnorderedAccess);
    ctx.BuildAccelerationStructure(*m_AccelerationStructure);
    ctx.PopMarker();
}

void Ether::Graphics::Mesh::InitializeVertexBufferViews()
{
    m_VertexBufferView = {};
    m_VertexBufferView.m_BufferSize = m_NumVertices * GetVertexStride();
    m_VertexBufferView.m_Stride = GetVertexStride();
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

void Ether::Graphics::Mesh::CreateGpuResources(CommandContext& ctx)
{
    CreateVertexBuffer(ctx);
    CreateIndexBuffer(ctx);
    CreateAccelerationStructure(ctx);
}

