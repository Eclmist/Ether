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

#include "rawmesh.h"

ETH_NAMESPACE_BEGIN

void RawMesh::CalculateNormals()
{
    const uint32_t numIndices = (uint32_t)m_Indices.size();
    m_Normals.reserve(numIndices);

    for (uint32_t i = 0; i < numIndices; i += 3)
    {
        const uint32_t i0 = m_Indices[i];
        const uint32_t i1 = m_Indices[i + 1];
        const uint32_t i2 = m_Indices[i + 2];

        ethXMVector v0 = XMLoadFloat3(&m_Positions[i0]);
        ethXMVector v1 = XMLoadFloat3(&m_Positions[i1]);
        ethXMVector v2 = XMLoadFloat3(&m_Positions[i2]);

        ethXMVector e1 = DirectX::XMVectorSubtract(v1, v0);
        ethXMVector e2 = DirectX::XMVectorSubtract(v2, v0);
        ethXMVector xmNormal = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(e1, e2));

        ethVector3 normal;
        DirectX::XMStoreFloat3(&normal, xmNormal);

        m_Normals.emplace_back(normal);
        m_Normals.emplace_back(normal);
        m_Normals.emplace_back(normal);
    }
}

void RawMesh::ProcessVertices()
{
    const uint32_t numVertices = (uint32_t)m_Positions.size();

    m_Vertices.reserve(numVertices);
    for (uint32_t i = 0; i < numVertices; ++i)
    {
        Vertex v;
		v.m_Position = (m_Positions.size() <= i) ? ethVector3() : m_Positions[i];
        v.m_Normal = (m_Normals.size() <= i) ? ethVector3() : m_Normals[i];
        v.m_Tangent = (m_Tangents.size() <= i) ? ethVector4() : m_Tangents[i];
        v.m_TexCoord = (m_TexCoords.size() <= i) ? ethVector2() : m_TexCoords[i];
        m_Vertices.emplace_back(v);
    }
}

void RawMesh::PackVertices()
{
    const uint32_t numVertices = (uint32_t)m_Vertices.size();
    assert(numVertices != 0);

    m_PackedData.reserve(numVertices);

    for (uint32_t i = 0; i < numVertices; ++i)
    {
        Vertex& v = m_Vertices[i];

        // TODO: Compress normals and tangents
        VertexFormats::VertexFormatStatic packedVertex;
        packedVertex.m_Position = m_Vertices[i].m_Position;
        packedVertex.m_Normal = m_Vertices[i].m_Normal;
        packedVertex.m_Tangent = m_Vertices[i].m_Tangent;
        packedVertex.m_UV = m_Vertices[i].m_TexCoord;
    }
}

void RawMesh::Format()
{
    AssertToolmode(m_Indices.size() != 0, "Cannot pack empty mesh");
    AssertToolmode((m_Indices.size() % 3) == 0, "Cannot pack non-triangle meshes");

    if (m_Normals.empty())
        CalculateNormals();

    ProcessVertices();
    PackVertices();
}

ETH_NAMESPACE_END
