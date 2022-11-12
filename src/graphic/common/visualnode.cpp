/*
    This file is part of Ether, an open-source DirectX 12 renderer.

    Copyright (c) 2020-2022 Samuel Huang - All rights reserved.

    Ether is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "visualnode.h"
#include "graphic/rhi/rhidevice.h"
#include "graphic/rhi/rhiresource.h"

ETH_NAMESPACE_BEGIN

VisualNode::VisualNode(const VisualNodeDesc desc)
    : m_NodeDesc(desc)
    , m_InstanceParams((L"Visual Node Instance Params " + std::to_wstring(desc.m_NodeId)).c_str())
{
    UploadVertexBuffer(m_NodeDesc.m_VertexBuffer, m_NodeDesc.m_NumVertices);
    InitVertexBufferView(m_NodeDesc.m_NumVertices * sizeof(VertexFormats::VertexFormatStatic), sizeof(VertexFormats::VertexFormatStatic));
    InitInstanceParams();
}

VisualNode::~VisualNode()
{
    m_VertexBuffer.Destroy();
    m_IndexBuffer.Destroy();
    m_InstanceParams->Unmap();
}

void VisualNode::UploadVertexBuffer(const void* data, uint32_t numVertices)
{
    size_t bufferSize = numVertices * sizeof(VertexFormats::VertexFormatStatic);
    RhiCommitedResourceDesc desc = {};
    desc.m_HeapType = RhiHeapType::Default;
    desc.m_State = RhiResourceState::Common;
    desc.m_ResourceDesc = RhiCreateBufferResourceDesc(bufferSize);

    GraphicCore::GetDevice()->CreateCommittedResource(desc, m_VertexBuffer);
    m_VertexBuffer->SetName(L"VisualNode::VertexBuffer");
    CommandContext::InitializeBufferTemp(m_VertexBuffer, data, bufferSize);
}

void VisualNode::InitVertexBufferView(size_t bufferSize, size_t stride)
{
    m_VertexBufferView = {};
    m_VertexBufferView.m_BufferSize = bufferSize;
    m_VertexBufferView.m_Stride = stride;
    m_VertexBufferView.m_Resource = m_VertexBuffer;
}

void VisualNode::InitInstanceParams()
{
    GraphicCore::GetGraphicRenderer().GetResourceContext().CreateConstantBuffer(sizeof(VisualNodeInstanceParams), m_InstanceParams);
    m_InstanceParams->Map((void**)&m_MappedParams);
}

ETH_NAMESPACE_END

