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

#include "mesh.h"

ETH_NAMESPACE_BEGIN

void Mesh::CalculateNormals()
{
    const uint32_t numIndices = (uint32_t)m_PositionIndices.size();
    m_Normals.resize(numIndices);
    m_NormalIndices = m_PositionIndices;

    for (uint32_t i = 0; i < numIndices; i += 3)
    {
        const uint32_t i0 = m_PositionIndices[i];
        const uint32_t i1 = m_PositionIndices[i + 1];
        const uint32_t i2 = m_PositionIndices[i + 2];

        ethXMVector v0 = XMLoadFloat3(&m_Positions[i0]);
        ethXMVector v1 = XMLoadFloat3(&m_Positions[i1]);
        ethXMVector v2 = XMLoadFloat3(&m_Positions[i2]);

        ethXMVector e1 = DirectX::XMVectorSubtract(v1, v0);
        ethXMVector e2 = DirectX::XMVectorSubtract(v2, v0);
        ethXMVector xmNormal = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(e1, e2));

        DirectX::XMStoreFloat3(&m_Normals[m_NormalIndices[i]], xmNormal);
        m_Normals[m_NormalIndices[i + 1]] = m_Normals[m_NormalIndices[i]];
        m_Normals[m_NormalIndices[i + 2]] = m_Normals[m_NormalIndices[i]];
    }
}

void Mesh::CalculateTexCoords()
{
    const uint32_t numIndices = (uint32_t)m_PositionIndices.size();
    m_TexCoords.resize(numIndices);
    m_TexCoordIndices.resize(numIndices);

	for (uint32_t i = 0; i < numIndices; ++i)
	{
        m_TexCoords[i] = ethVector2();
		m_TexCoordIndices[i] = 0;
    }
}

void Mesh::GenerateVertices()
{
    const uint32_t numIndices = (uint32_t)m_PositionIndices.size();
    const uint32_t numPositions = (uint32_t)m_Positions.size();

    m_Indices.resize(numIndices);
    std::vector<std::vector<uint32_t>> indexRef(numPositions);

    for (uint32_t i = 0; i < numIndices; ++i)
        indexRef[m_PositionIndices[i]].emplace_back(i);

    for (uint32_t i = 0; i < numPositions; ++i)
    {
        ethVector3 position = m_Positions[i];
        std::vector<Vertex> uniqueVerts;

        for (uint32_t j = 0; j < indexRef[i].size(); ++j)
        {
            ethVector3 normal = m_Normals[m_NormalIndices[indexRef[i][j]]];
            ethVector2 texcoord = m_TexCoords[m_TexCoordIndices[indexRef[i][j]]];
            Vertex newVertex;
            newVertex.m_Position = position;
            newVertex.m_Normal = normal;
            newVertex.m_TexCoord = texcoord;

            bool vertexExists = false;
            uint32_t vertexIndex = -1;

            for (uint32_t k = 0; k < uniqueVerts.size(); ++k)
            {
                if (newVertex == uniqueVerts[k])
                {
                    vertexExists = true;
                    vertexIndex = k;
                    break;
                }
            }

            if (vertexExists)
            {
                vertexIndex = m_Vertices.size() - uniqueVerts.size() + vertexIndex;
            }
            else 
            {
                uniqueVerts.emplace_back(newVertex);
                m_Vertices.emplace_back(newVertex);
                vertexIndex = m_Vertices.size() - 1;
            }

			m_Indices[indexRef[i][j]] = vertexIndex;
        }
    }
}

void Mesh::PackVertices()
{
    const uint32_t numVertices = (uint32_t)m_Vertices.size();
    assert(numVertices != 0);

    m_PackedVertexData.resize(numVertices);

    for (uint32_t i = 0; i < numVertices; ++i)
    {
        Vertex& v = m_Vertices[i];

        // TODO: Compress normals and tangents
        VertexFormats::VertexFormatStatic packedVertex;
        packedVertex.m_Position = m_Vertices[i].m_Position;
        packedVertex.m_Normal = m_Vertices[i].m_Normal;
        packedVertex.m_Tangent = ethVector4();
        packedVertex.m_UV = m_Vertices[i].m_TexCoord;
        m_PackedVertexData[i] = packedVertex;
    }
}

void Mesh::Compile()
{
    AssertToolmode(m_PositionIndices.size() != 0, "Cannot pack empty mesh");
    AssertToolmode((m_PositionIndices.size() % 3) == 0, "Cannot pack non-triangle meshes");

    if (m_Normals.empty())
        CalculateNormals();

    if (m_TexCoords.empty())
        CalculateTexCoords();

    GenerateVertices();
    PackVertices();
}

ETH_NAMESPACE_END

