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

#pragma once

ETH_NAMESPACE_BEGIN

struct VisualNodeData
{
    const void* m_VertexBuffer;
    const void* m_IndexBuffer;

    uint32_t m_NumVertices;
    uint32_t m_NumIndices;

    const ethXMMatrix* m_ModelMatrix;
};

class VisualNode
{
public:
    VisualNode(const VisualNodeData data);
    ~VisualNode();

    inline RHIVertexBufferViewDesc GetVertexBufferView() const { return m_VertexBufferView; }
    inline RHIIndexBufferViewDesc GetIndexBufferView() const { return m_IndexBufferView; }
    inline uint32_t GetNumIndices() const { return m_StaticData.m_NumIndices; }
    inline const ethXMMatrix GetModelMatrix() const { return *m_StaticData.m_ModelMatrix; }

private:
    void UploadVertexBuffer(const void* data, uint32_t numVertices);
    void UploadIndexBuffer(const void* data, uint32_t numIndices);

    void InitVertexBufferView(size_t bufferSize, size_t stride);
    void InitIndexBufferView(size_t bufferSize);

private:
    RHIResourceHandle m_VertexBuffer;
    RHIResourceHandle m_IndexBuffer;

    RHIVertexBufferViewDesc m_VertexBufferView;
    RHIIndexBufferViewDesc m_IndexBufferView;

    VisualNodeData m_StaticData;
};

ETH_NAMESPACE_END

