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

ETH_NAMESPACE_BEGIN

Visual::Visual()
{
    InitializeDebugVertexData();
    CreateVertexBuffer();
    CreateIndexBuffer();
}

void Visual::InitializeDebugVertexData()
{
    m_NumVertices = 36;

    m_VertexBuffer[0] = { ethVector3(-1.0f, -1.0f, -1.0f), ethVector3(0.0f, 0.0f, 0.0f) };
    m_VertexBuffer[1] = { ethVector3(-1.0f,  1.0f, -1.0f), ethVector3(0.0f, 1.0f, 0.0f) };
    m_VertexBuffer[2] = { ethVector3(1.0f,  1.0f, -1.0f), ethVector3(1.0f, 1.0f, 0.0f) };
    m_VertexBuffer[3] = { ethVector3(1.0f, -1.0f, -1.0f), ethVector3(1.0f, 0.0f, 0.0f) };
    m_VertexBuffer[4] = { ethVector3(-1.0f, -1.0f,  1.0f), ethVector3(0.0f, 0.0f, 1.0f) };
    m_VertexBuffer[5] = { ethVector3(-1.0f,  1.0f,  1.0f), ethVector3(0.0f, 1.0f, 1.0f) };
    m_VertexBuffer[6] = { ethVector3(1.0f,  1.0f,  1.0f), ethVector3(1.0f, 1.0f, 1.0f) };
    m_VertexBuffer[7] = { ethVector3(1.0f, -1.0f,  1.0f), ethVector3(1.0f, 0.0f, 1.0f) };

    uint16_t indices[] = {
        0, 1, 2, 0, 2, 3,
        4, 6, 5, 4, 7, 6,
        4, 5, 1, 4, 1, 0,
        3, 2, 6, 3, 6, 7,
        1, 5, 6, 1, 6, 2,
        4, 0, 3, 4, 3, 7
    };

    memcpy(m_IndexBuffer, indices, sizeof(uint16_t) * m_NumVertices);
}

void Visual::CreateVertexBuffer()
{
    const size_t bufferSize = sizeof(m_VertexBuffer);

    ASSERT_SUCCESS(g_GraphicDevice->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(bufferSize, D3D12_RESOURCE_FLAG_NONE),
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        IID_PPV_ARGS(&m_VertexBufferResource)));

    ASSERT_SUCCESS(g_GraphicDevice->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(bufferSize),
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&m_IVertexBufferResource)));

}

void Visual::CreateIndexBuffer()
{
    const size_t bufferSize = sizeof(m_IndexBuffer);

    ASSERT_SUCCESS(g_GraphicDevice->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(bufferSize, D3D12_RESOURCE_FLAG_NONE),
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        IID_PPV_ARGS(&m_IndexBufferResource)));

    ASSERT_SUCCESS(g_GraphicDevice->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(bufferSize),
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&m_IIndexBufferResource)));
}

void Visual::UploadVertexBuffer()
{
    const size_t bufferSize = sizeof(m_VertexBuffer);

    ID3D12GraphicsCommandList* initCmdList;
    ID3D12CommandAllocator* allocator;
    g_CommandManager.CreateCommandList(D3D12_COMMAND_LIST_TYPE_COPY, &initCmdList, &allocator);

    D3D12_SUBRESOURCE_DATA subresourceData = {};
    subresourceData.pData = m_VertexBuffer;
    subresourceData.RowPitch = bufferSize;
    subresourceData.SlicePitch = subresourceData.RowPitch;

    ASSERT_SUCCESS(UpdateSubresources(initCmdList, m_VertexBufferResource.Get(), m_IVertexBufferResource.Get(),
        0, 0, 1, &subresourceData));

    m_VertexBufferView.BufferLocation = m_VertexBufferResource->GetGPUVirtualAddress();
    m_VertexBufferView.SizeInBytes = sizeof(m_VertexBuffer);
    m_VertexBufferView.StrideInBytes = sizeof(VertexPositionColor);
}

void Visual::UploadIndexBuffer()
{
    const size_t bufferSize = sizeof(m_IndexBuffer);

    ID3D12GraphicsCommandList* initCmdList;
    ID3D12CommandAllocator* allocator;
    g_CommandManager.CreateCommandList(D3D12_COMMAND_LIST_TYPE_COPY, &initCmdList, &allocator);

    D3D12_SUBRESOURCE_DATA subresourceData = {};
    subresourceData.pData = m_IndexBuffer;
    subresourceData.RowPitch = bufferSize;
    subresourceData.SlicePitch = subresourceData.RowPitch;

    ASSERT_SUCCESS(UpdateSubresources(initCmdList, m_IndexBufferResource.Get(), m_IIndexBufferResource.Get(),
        0, 0, 1, &subresourceData));

    m_VertexBufferView.BufferLocation = m_IndexBufferResource->GetGPUVirtualAddress();
    m_VertexBufferView.SizeInBytes = sizeof(m_IndexBuffer);
    m_VertexBufferView.StrideInBytes = sizeof(uint16_t);
}


ETH_NAMESPACE_END

