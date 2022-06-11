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

#include "asset.h"

#ifdef ETH_TOOLMODE
#include "toolmode/asset/intermediate/mesh.h"
#endif

ETH_NAMESPACE_BEGIN

#define MAX_VERTICES 65536 

class CompiledMesh : public Asset
{
public:
    CompiledMesh() = default;
    ~CompiledMesh() = default;

    void Serialize(OStream& ostream) override;
    void Deserialize(IStream& istream) override;

public:
    inline uint32_t GetNumVertices() const { return m_NumVertices; }
    inline uint32_t GetNumIndices() const { return m_IndexBuffer.size(); }

    inline const void* GetVertexBuffer() const { return m_VertexBuffer.data(); }
    inline const uint32_t* GetIndexBuffer() const { return m_IndexBuffer.data(); }

    inline bool IsValid() const { return GetNumVertices() > 0; }

public:
    ETH_TOOLONLY(void SetRawMesh(std::shared_ptr<Mesh> rawMesh));

private:
    ETH_TOOLONLY(void SetVertexBuffer(char* vertices, size_t size));
    ETH_TOOLONLY(void SetIndexBuffer(uint32_t* indices, size_t size));
    ETH_TOOLONLY(void UpdateBuffers());

private:
    std::vector<char> m_VertexBuffer;
    std::vector<uint32_t> m_IndexBuffer;

    uint32_t m_NumVertices;

    ETH_TOOLONLY(std::shared_ptr<Mesh> m_RawMesh);
};

ETH_NAMESPACE_END

