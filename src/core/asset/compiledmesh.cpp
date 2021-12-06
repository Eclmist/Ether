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

#include "compiledmesh.h"

ETH_NAMESPACE_BEGIN

CompiledMesh::CompiledMesh()
    : m_VertexBuffer(nullptr)
    , m_VertexBufferSize(0)
    , m_NumVertices(0)
    , m_NumIndices(0)
{
}

void CompiledMesh::Serialize(OStream& ostream)
{
    Asset::Serialize(ostream);

    ostream << m_VertexBufferSize;
    ostream << m_NumVertices;
    ostream << m_NumIndices;

    for (int i = 0; i < m_VertexBufferSize; ++i)
        ostream << reinterpret_cast<char*>(m_VertexBuffer)[i];

    for (int i = 0; i < m_NumIndices; ++i)
        ostream << m_IndexBuffer[i];
}

void CompiledMesh::Deserialize(IStream& istream)
{
    Asset::Deserialize(istream);

    istream >> m_VertexBufferSize;
    istream >> m_NumVertices;
    istream >> m_NumIndices;

    m_VertexBuffer = malloc(m_VertexBufferSize);
    for (int i = 0; i < m_VertexBufferSize; ++i)
        istream >> reinterpret_cast<char*>(m_VertexBuffer)[i];

    for (int i = 0; i < m_NumIndices; ++i)
        istream >> m_IndexBuffer[i];
}

#ifdef ETH_TOOLMODE

void CompiledMesh::SetRawMesh(std::shared_ptr<Mesh> rawMesh)
{
    m_RawMesh = rawMesh;
    UpdateBuffers();
}

void CompiledMesh::SetVertexBuffer(void* vertices, size_t size)
{
    if (m_VertexBuffer != nullptr)
        free(m_VertexBuffer);

    m_VertexBuffer = malloc(size);
    m_VertexBufferSize = size;
    memcpy(m_VertexBuffer, vertices, size);
}

void CompiledMesh::SetIndexBuffer(uint32_t* indices, size_t size)
{
    memcpy(m_IndexBuffer, indices, size);
}

void CompiledMesh::ClearBuffers()
{
    memset(m_VertexBuffer, 0, m_VertexBufferSize);
    memset(m_IndexBuffer, 0, sizeof(m_IndexBuffer));
    m_VertexBuffer = nullptr;
	m_VertexBufferSize = 0;
	m_NumVertices = 0;
    m_NumIndices = 0;
}

void CompiledMesh::UpdateBuffers()
{
    if (m_RawMesh->GetNumIndices() >= MAX_VERTICES)
    {
		LogToolmodeError("Failed to load mesh asset - max vertex count exceeded");
        ClearBuffers();
        return;
    }

    SetVertexBuffer(m_RawMesh->GetPackedVertexData(), m_RawMesh->GetPackedVertexDataSize());
    SetIndexBuffer(m_RawMesh->GetIndices(), m_RawMesh->GetIndicesSize());
    m_NumVertices = m_RawMesh->GetNumVertices();
    m_NumIndices = m_RawMesh->GetNumIndices();
}

#endif // ETH_TOOLMODE

ETH_NAMESPACE_END

