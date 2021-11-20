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

#include "meshasset.h"

ETH_NAMESPACE_BEGIN

MeshAsset::MeshAsset()
    : m_VertexBuffer(nullptr)
    , m_VertexBufferSize(0)
    , m_NumVertices(0)
    , m_NumIndices(0)
{
    m_Type = ASSETTYPE_MESH;
}

#ifdef ETH_TOOLMODE
void MeshAsset::UpdateBuffers()
{
    SetVertexBuffer(m_RawMesh.m_Vertices.data(), m_RawMesh.m_Vertices.size() * sizeof(m_RawMesh.m_Vertices[0]));
    SetIndexBuffer(m_RawMesh.m_Indices.data(), m_RawMesh.m_Indices.size() * sizeof(m_RawMesh.m_Indices[0]));
    m_NumVertices = m_RawMesh.m_Vertices.size();
    m_NumIndices = m_RawMesh.m_Indices.size();

}

void MeshAsset::SetVertexBuffer(void* vertices, size_t size)
{
    if (m_VertexBuffer != nullptr)
        free(m_VertexBuffer);

    m_VertexBuffer = malloc(size);
    m_VertexBufferSize = size;
    memcpy(m_VertexBuffer, vertices, size);
}

void MeshAsset::SetIndexBuffer(uint32_t indices[], size_t size)
{
    memcpy(m_IndexBuffer, indices, size);
}
#endif // ETH_TOOLMODE

ETH_NAMESPACE_END

