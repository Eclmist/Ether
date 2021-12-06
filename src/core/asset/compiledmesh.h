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

#include "asset.h"

#ifdef ETH_TOOLMODE
#include "toolmode/asset/intermediate/mesh.h"
#endif

ETH_NAMESPACE_BEGIN

// Arbitrary max number of vertices
#define MAX_VERTICES 655360 

class CompiledMesh : public Asset
{
public:
    CompiledMesh();
    ~CompiledMesh() = default;

    void Serialize(OStream& ostream) override;
    void Deserialize(IStream& istream) override;

public:
    inline uint32_t GetNumVertices() const { return m_NumVertices; }
    inline uint32_t GetNumIndices() const { return m_NumIndices; }

    inline void* GetVertexBuffer() const { return m_VertexBuffer; }
    inline size_t GetVertexBufferSize() const { return m_VertexBufferSize; }
    inline uint32_t* GetIndexBuffer() const { return (uint32_t*)m_IndexBuffer; }

    inline bool IsValid() const { return m_VertexBuffer != nullptr; }

public:
    ETH_TOOLONLY(void SetRawMesh(std::shared_ptr<Mesh> rawMesh));

private:
    ETH_TOOLONLY(void SetVertexBuffer(void* vertices, size_t size));
    ETH_TOOLONLY(void SetIndexBuffer(uint32_t* indices, size_t size));
    ETH_TOOLONLY(void ClearBuffers());
    ETH_TOOLONLY(void UpdateBuffers());

private:
    void* m_VertexBuffer;
    uint32_t m_VertexBufferSize;
    uint32_t m_IndexBuffer[MAX_VERTICES];

    uint32_t m_NumVertices;
    uint32_t m_NumIndices;

    ETH_TOOLONLY(std::shared_ptr<Mesh> m_RawMesh);
};

ETH_NAMESPACE_END

