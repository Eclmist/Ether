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

#include "compiledmesh.h"

ETH_NAMESPACE_BEGIN

void CompiledMesh::Serialize(OStream& ostream)
{
    Asset::Serialize(ostream);

    ostream << (uint32_t)m_VertexBuffer.size();
    ostream << m_NumVertices;

    for (int i = 0; i < m_VertexBuffer.size(); ++i)
        ostream << m_VertexBuffer[i];

    m_Material->Serialize(ostream);
}

void CompiledMesh::Deserialize(IStream& istream)
{
    Asset::Deserialize(istream);

    uint32_t vertexBufferSize;

    istream >> vertexBufferSize;
    istream >> m_NumVertices;

    m_VertexBuffer.resize(vertexBufferSize);

    for (int i = 0; i < vertexBufferSize; ++i)
        istream >> m_VertexBuffer[i];

    m_Material = std::make_shared<Material>();
    m_Material->Deserialize(istream);
}

#ifdef ETH_TOOLMODE

void CompiledMesh::SetRawMesh(std::shared_ptr<RawMesh> rawMesh)
{
    m_RawMesh = rawMesh;
    UpdateBuffers();
    UpdateMaterial();
}

void CompiledMesh::SetVertexBuffer(char* vertices, size_t size)
{
    m_VertexBuffer.resize(size);
    m_VertexBuffer.assign(vertices, vertices + size);
}

void CompiledMesh::UpdateBuffers()
{
    if (m_RawMesh->GetNumVertices() >= MAX_VERTICES)
        LogToolmodeWarning("Max recommended vertex count exceeded. Memory exceptions may occur.");

    SetVertexBuffer(static_cast<char*>(m_RawMesh->GetPackedVertexData()), m_RawMesh->GetPackedVertexDataSize());
    m_NumVertices = m_RawMesh->GetNumVertices();
}

void CompiledMesh::UpdateMaterial()
{
    m_Material = m_RawMesh->GetMaterial();
}


#endif // ETH_TOOLMODE

ETH_NAMESPACE_END

