/*
    This file is part of Ether, an open-source DirectX 12 renderer.

    Copyright (c) 2020-2030 Samuel Huang - All rights reserved.

    Ether is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "graphics/resources/staticmesh.h"
#include "graphics/graphiccore.h"

constexpr uint32_t StaticMeshVersion = 10;

Ether::Graphics::StaticMesh::StaticMesh()
    : Mesh(StaticMeshVersion, ETH_CLASS_ID_STATICMESH)
{
}

void Ether::Graphics::StaticMesh::Serialize(OStream& ostream) const
{
    Mesh::Serialize(ostream);

    ostream << m_NumVertices;
    for (int i = 0; i < m_PackedVertices.size(); ++i)
        m_PackedVertices[i].Serialize(ostream);
}

void Ether::Graphics::StaticMesh::Deserialize(IStream& istream)
{
    Mesh::Deserialize(istream);

    istream >> m_NumVertices;
    AssertGraphics(m_NumVertices <= MaxVerticesPerMesh, "Num vertices exceeds limit");

    m_PackedVertices.resize(m_NumVertices);
    for (int i = 0; i < m_PackedVertices.size(); ++i)
        m_PackedVertices[i].Deserialize(istream);
}

void Ether::Graphics::StaticMesh::CreateGpuResources(CommandContext& ctx)
{
    Mesh::CreateGpuResources(ctx);

#ifdef ETH_ENGINE
    // Mesh data can be deallocated on the CPU. It's all in VRAM now.
    // This might cause problems down the line, but if it is not deallocated the CPU memory usage is going to be crazy
    m_PackedVertices.clear();
    m_Indices.clear();
#endif
}

void Ether::Graphics::StaticMesh::ComputeBoundingBox()
{
    m_BoundingBox.m_Min = 9999999;
    m_BoundingBox.m_Max = -9999999;

    for (auto& vertex : m_PackedVertices)
    {
        m_BoundingBox.m_Min.x = std::min(m_BoundingBox.m_Min.x, vertex.m_Attributes.m_Position.x);
        m_BoundingBox.m_Min.y = std::min(m_BoundingBox.m_Min.y, vertex.m_Attributes.m_Position.y);
        m_BoundingBox.m_Min.z = std::min(m_BoundingBox.m_Min.z, vertex.m_Attributes.m_Position.z);
    
        m_BoundingBox.m_Max.x = std::max(m_BoundingBox.m_Max.x, vertex.m_Attributes.m_Position.x);
        m_BoundingBox.m_Max.y = std::max(m_BoundingBox.m_Max.y, vertex.m_Attributes.m_Position.y);
        m_BoundingBox.m_Max.z = std::max(m_BoundingBox.m_Max.z, vertex.m_Attributes.m_Position.z);
    }
}

void Ether::Graphics::StaticMesh::SetPackedVertices(std::vector<VertexFormats::BaseVertexFormat>&& vertices)
{
    m_PackedVertices = std::move(vertices);
    m_NumVertices = m_PackedVertices.size();

    ComputeBoundingBox();
}
