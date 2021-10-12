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

ETH_NAMESPACE_BEGIN

VisualNode::VisualNode(const VisualNodeData data)
    : m_StaticData(data)
{
    UploadVertexBuffer(m_StaticData.m_VertexBuffer, m_StaticData.m_NumVertices);
    UploadIndexBuffer(m_StaticData.m_IndexBuffer, m_StaticData.m_NumIndices);

    InitVertexBufferView(m_StaticData.m_NumVertices * sizeof(VertexPositionColor), sizeof(VertexPositionColor));
    InitIndexBufferView(m_StaticData.m_NumIndices * sizeof(uint16_t));
}

void VisualNode::UploadVertexBuffer(const void* data, uint16_t numVertices)
{
    m_VertexBuffer = std::make_unique<BufferResource>(L"Visual::VertexBuffer",
        numVertices, sizeof(VertexPositionColor), data);
}

void VisualNode::UploadIndexBuffer(const void* data, uint16_t numIndices)
{ 
    m_IndexBuffer = std::make_unique<BufferResource>(L"Visual::IndexBuffer",
        numIndices, sizeof(uint16_t), data);
}

void VisualNode::InitVertexBufferView(size_t bufferSize, size_t stride)
{
    m_VertexBufferView = {};
    m_VertexBufferView.BufferLocation = m_VertexBuffer->GetVirtualAddress();
    m_VertexBufferView.SizeInBytes = bufferSize;
    m_VertexBufferView.StrideInBytes = stride;
}

void VisualNode::InitIndexBufferView(size_t bufferSize)
{
    m_IndexBufferView = {};
    m_IndexBufferView.BufferLocation = m_IndexBuffer->GetVirtualAddress();
    m_IndexBufferView.Format = DXGI_FORMAT_R16_UINT;
    m_IndexBufferView.SizeInBytes = bufferSize;
}

ETH_NAMESPACE_END

