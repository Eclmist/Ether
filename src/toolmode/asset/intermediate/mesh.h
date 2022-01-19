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

#include "graphic/common/vertexformat.h"

ETH_NAMESPACE_BEGIN

class CompiledMesh;

struct Vertex
{
    ethVector3 m_Position;
    ethVector3 m_Normal;
    ethVector2 m_TexCoord;

    bool operator==(const Vertex& other)
    {
        if (m_Position.x != other.m_Position.x ||
            m_Position.y != other.m_Position.y ||
            m_Position.z != other.m_Position.z)
            return false;

        if (m_Normal.x != other.m_Normal.x ||
            m_Normal.y != other.m_Normal.y ||
            m_Normal.z != other.m_Normal.z)
            return false;

        if (m_TexCoord.x != other.m_TexCoord.x ||
            m_TexCoord.y != other.m_TexCoord.y)
            return false;

        return true;
    }
};

class Mesh : public Asset
{
public:
    Mesh() = default;
    ~Mesh() = default;

public:
    void Compile();

    inline void* GetPackedVertexData() const { return (void*)m_PackedVertexData.data(); }
    inline size_t GetPackedVertexDataSize() const { return m_PackedVertexData.size() * sizeof(m_PackedVertexData[0]); }
    inline uint32_t* GetIndices() const { return (uint32_t*)m_Indices.data(); }
    inline size_t GetIndicesSize() const { return m_Indices.size() * sizeof(m_Indices[0]); }

    inline uint32_t GetNumVertices() const { return m_PackedVertexData.size(); }
    inline uint32_t GetNumIndices() const { return m_Indices.size(); }

private:
    friend class ObjFileParser;

    void CalculateNormals();
    void CalculateTexCoords();
    void GenerateVertices();
    void PackVertices();

private:
    std::vector<ethVector3> m_Positions;
    std::vector<ethVector3> m_Normals;
    std::vector<ethVector2> m_TexCoords;

    std::vector<uint32_t> m_PositionIndices;
    std::vector<uint32_t> m_NormalIndices;
    std::vector<uint32_t> m_TexCoordIndices;

    std::vector<Vertex> m_Vertices;

    std::vector<VertexFormats::VertexFormatStatic> m_PackedVertexData;
    std::vector<uint32_t> m_Indices;
};

ETH_NAMESPACE_END

