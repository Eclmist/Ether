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

CompiledMeshAsset::CompiledMeshAsset()
    : m_VertexBuffer(nullptr)
    , m_VertexBufferSize(0)
    , m_NumVertices(0)
    , m_NumIndices(0)
{
    m_Type = ASSETTYPE_MESH;
}

#ifdef ETH_TOOLMODE

void CompiledMeshAsset::SetRawMesh(std::shared_ptr<RawMeshAsset> rawMesh)
{
    m_RawMesh = rawMesh;
    UpdateBuffers();
}

void CompiledMeshAsset::SetVertexBuffer(void* vertices, size_t size)
{
    if (m_VertexBuffer != nullptr)
        free(m_VertexBuffer);

    m_VertexBuffer = malloc(size);
    m_VertexBufferSize = size;
    memcpy(m_VertexBuffer, vertices, size);
}

void CompiledMeshAsset::SetIndexBuffer(uint32_t* indices, size_t size)
{
    memcpy(m_IndexBuffer, indices, size);
}

void CompiledMeshAsset::ClearBuffers()
{
    memset(m_VertexBuffer, 0, m_VertexBufferSize);
    memset(m_IndexBuffer, 0, sizeof(m_IndexBuffer));
    m_VertexBuffer = nullptr;
	m_VertexBufferSize = 0;
	m_NumVertices = 0;
    m_NumIndices = 0;
}

void CompiledMeshAsset::UpdateBuffers()
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

