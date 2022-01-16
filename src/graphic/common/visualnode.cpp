/*
    This file is part of Ether, an open-source DirectX 12 renderer.

    Copyright (c) 2020-2021 Samuel Van Allen - All rights reserved.

    Ether is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "visualnode.h"
#include "graphic/rhi/rhidevice.h"
#include "graphic/rhi/rhiresource.h"

ETH_NAMESPACE_BEGIN

VisualNode::VisualNode(const VisualNodeData data)
    : m_StaticData(data)
{
    UploadVertexBuffer(m_StaticData.m_VertexBuffer, m_StaticData.m_NumVertices);
    UploadIndexBuffer(m_StaticData.m_IndexBuffer, m_StaticData.m_NumIndices);

    InitVertexBufferView(m_StaticData.m_NumVertices * sizeof(VertexFormats::VertexFormatStatic), sizeof(VertexFormats::VertexFormatStatic));
    InitIndexBufferView(m_StaticData.m_NumIndices * sizeof(uint32_t));
}

VisualNode::~VisualNode()
{
    m_VertexBuffer.Destroy();
    m_IndexBuffer.Destroy();
}

void VisualNode::UploadVertexBuffer(const void* data, uint32_t numVertices)
{
    size_t bufferSize = numVertices * sizeof(VertexFormats::VertexFormatStatic);
    RHICommitedResourceDesc desc = {};
    desc.m_HeapType = RHIHeapType::Default;
    desc.m_State = RHIResourceState::Common;
    desc.m_ResourceDesc = RHICreateBufferResourceDesc(bufferSize);

    GraphicCore::GetDevice()->CreateCommittedResource(desc, m_VertexBuffer);
    m_VertexBuffer->SetName(L"VisualNode::VertexBuffer");
    CommandContext::InitializeBuffer(m_VertexBuffer, data, bufferSize);
}

void VisualNode::UploadIndexBuffer(const void* data, uint32_t numIndices)
{ 
    size_t bufferSize = numIndices * sizeof(uint32_t);
    RHICommitedResourceDesc desc = {};
    desc.m_HeapType = RHIHeapType::Default;
    desc.m_State = RHIResourceState::Common;
    desc.m_ResourceDesc = RHICreateBufferResourceDesc(bufferSize);

    GraphicCore::GetDevice()->CreateCommittedResource(desc, m_IndexBuffer);
    m_IndexBuffer->SetName(L"VisualNode::IndexBuffer");
    CommandContext::InitializeBuffer(m_IndexBuffer, data, bufferSize);
}

void VisualNode::InitVertexBufferView(size_t bufferSize, size_t stride)
{
    m_VertexBufferView = {};
    m_VertexBufferView.m_BufferSize = bufferSize;
    m_VertexBufferView.m_Stride = stride;
    m_VertexBufferView.m_Resource = m_VertexBuffer;
}

void VisualNode::InitIndexBufferView(size_t bufferSize)
{
    m_IndexBufferView = {};
    m_IndexBufferView.m_BufferSize = bufferSize;
    m_IndexBufferView.m_Format = RHIFormat::R32Uint;
    m_IndexBufferView.m_Resource = m_IndexBuffer;
}

ETH_NAMESPACE_END

