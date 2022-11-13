/*
    This file is part of Ether, an open-source DirectX 12 renderer.

    Copyright (c) 2020-2023 Samuel Huang - All rights reserved.

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

#pragma once

#include "graphic/common/vertexformat.h"

ETH_NAMESPACE_BEGIN

class Material;

struct Vertex
{
    Vertex(ethVector3 pos, ethVector3 norm, ethVector2 uv)
        : m_Position(pos)
        , m_Normal(norm)
        , m_TexCoord(uv) {}

    ethVector3 m_Position;
    ethVector3 m_Normal;
    ethVector2 m_TexCoord;

    bool operator==(const Vertex& other)
    {
        if (m_Position != other.m_Position)
            return false;

        if (m_Normal != other.m_Normal)
            return false;

        if (m_TexCoord != other.m_TexCoord)
            return false;

        return true;
    }
};

class RawMesh
{
public:
    RawMesh() = default;
    ~RawMesh() = default;

public:
    inline void* GetPackedVertexData() const { return (void*)m_PackedVertexData.data(); }
    inline size_t GetPackedVertexDataSize() const { return m_PackedVertexData.size() * sizeof(m_PackedVertexData[0]); }
    inline uint32_t GetNumVertices() const { return m_PackedVertexData.size(); }
    inline std::shared_ptr<Material> GetMaterial() const { return m_Material; }

private:
    friend class ObjFileParser;
    friend class MeshGroup;
    void Compile();

private:
    void CalculateNormals();
    void CalculateTexCoords();
    void GenerateVertices();
    void PackVertices();

private:
    std::vector<ethVector3> m_Positions;
    std::vector<ethVector3> m_Normals;
    std::vector<ethVector2> m_TexCoords;

    //std::vector<uint32_t> m_PositionIndices;
    //std::vector<uint32_t> m_NormalIndices;
    //std::vector<uint32_t> m_TexCoordIndices;

    std::vector<Vertex> m_Vertices;
    std::vector<VertexFormats::VertexFormatStatic> m_PackedVertexData;
    //std::vector<uint32_t> m_Indices;
    std::shared_ptr<Material> m_Material;
};

class MeshGroup : public Asset
{
public:
    MeshGroup() = default;
    ~MeshGroup() = default;

public:
    inline std::vector<std::shared_ptr<RawMesh>> GetSubMeshes() const { return m_SubMeshes; }

public:
    void Compile();

private:     
    friend class ObjFileParser;

private:
    std::vector<std::shared_ptr<RawMesh>> m_SubMeshes;
    std::vector<std::shared_ptr<Material>> m_MaterialTable;
};


ETH_NAMESPACE_END

