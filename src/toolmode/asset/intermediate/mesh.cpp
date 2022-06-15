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

#include "mesh.h"

ETH_NAMESPACE_BEGIN

void MeshGroup::Compile()
{
    for (std::shared_ptr<RawMesh> rawMesh : m_SubMeshes)
        rawMesh->Compile();
}

void RawMesh::CalculateNormals()
{
    m_Normals.resize(m_Positions.size());
    for (uint32_t i = 0; i < m_Positions.size(); i += 3)
    {
        ethVector3 v0 = m_Positions[i];
        ethVector3 v1 = m_Positions[i + 1];
        ethVector3 v2 = m_Positions[i + 2];

        ethVector3 e1 = v1 - v0;
        ethVector3 e2 = v2 - v0;
        ethVector3 normal = ethVector3::Cross(e1, e2);

        m_Normals[i] = normal;
        m_Normals[i + 1] = normal;
        m_Normals[i + 2] = normal;
    }
}

void RawMesh::CalculateTexCoords()
{
    m_TexCoords.resize(m_Positions.size());

	for (uint32_t i = 0; i < m_Positions.size(); ++i)
        m_TexCoords[i] = { 0, 0 };
}

void RawMesh::GenerateVertices()
{
    for (uint32_t i = 0; i < m_Positions.size(); ++i)
    {
        m_Vertices.emplace_back(
            m_Positions[i],
            m_Normals[i],
            m_TexCoords[i]
		);
    }
}

void RawMesh::PackVertices()
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

void RawMesh::Compile()
{
    AssertToolmode(m_Positions.size() != 0, "Cannot pack empty mesh");
    AssertToolmode((m_Positions.size() % 3) == 0, "Cannot pack non-triangle meshes");

	if (m_Normals.empty())
		CalculateNormals();

	if (m_TexCoords.empty())
		CalculateTexCoords();

    GenerateVertices();
    PackVertices();
}

ETH_NAMESPACE_END

