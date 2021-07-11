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

#pragma once

ETH_NAMESPACE_BEGIN

class MeshComponent;

class Visual
{
public:
    void Initialize(const MeshComponent& mesh);

    inline D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView() const { return m_VertexBufferView; }
    inline D3D12_INDEX_BUFFER_VIEW GetIndexBufferView() const { return m_IndexBufferView; }

private:
    void UploadVertexBuffer(const void* data, uint16_t numVertices);
    void UploadIndexBuffer(const void* data, uint16_t numIndices);

    void InitVertexBufferView(size_t bufferSize, size_t stride);
    void InitIndexBufferView(size_t bufferSize);

private:
    std::shared_ptr<BufferResource> m_VertexBuffer;
    std::shared_ptr<BufferResource> m_IndexBuffer;

    D3D12_VERTEX_BUFFER_VIEW m_VertexBufferView;
    D3D12_INDEX_BUFFER_VIEW m_IndexBufferView;
};

ETH_NAMESPACE_END

