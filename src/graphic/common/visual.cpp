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

#include "visual.h"
#include "core/entity/component/meshcomponent.h"

ETH_NAMESPACE_BEGIN

void Visual::Initialize(const MeshComponent& mesh)
{
    UploadVertexBuffer(mesh.m_VertexBuffer, mesh.m_NumVertices);
    UploadIndexBuffer(mesh.m_IndexBuffer, mesh.m_NumIndices);

    InitVertexBufferView(mesh.m_NumVertices * sizeof(VertexPositionColor), sizeof(VertexPositionColor));
    InitIndexBufferView(mesh.m_NumIndices * sizeof(uint16_t));
}

void Visual::UploadVertexBuffer(const void* data, uint16_t numVertices)
{
    m_VertexBuffer = std::make_shared<BufferResource>(L"Visual::VertexBuffer",
        numVertices, sizeof(VertexPositionColor), data);
}

void Visual::UploadIndexBuffer(const void* data, uint16_t numIndices)
{ 
    m_IndexBuffer = std::make_shared<BufferResource>(L"Visual::IndexBuffer",
        numIndices, sizeof(uint16_t), data);
}

void Visual::InitVertexBufferView(size_t bufferSize, size_t stride)
{
    m_VertexBufferView = {};
    m_VertexBufferView.BufferLocation = m_VertexBuffer->GetVirtualAddress();
    m_VertexBufferView.SizeInBytes = bufferSize;
    m_VertexBufferView.StrideInBytes = stride;
}

void Visual::InitIndexBufferView(size_t bufferSize)
{
    m_IndexBufferView = {};
    m_IndexBufferView.BufferLocation = m_IndexBuffer->GetVirtualAddress();
    m_IndexBufferView.Format = DXGI_FORMAT_R16_UINT;
    m_IndexBufferView.SizeInBytes = bufferSize;
}

ETH_NAMESPACE_END

