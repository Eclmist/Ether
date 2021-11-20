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

#pragma once

#include "graphic/common/vertexformat.h"

ETH_NAMESPACE_BEGIN

struct Vertex
{
    ethVector3 m_Position;
    ethVector3 m_Normal;
    ethVector4 m_Tangent;
    ethVector2 m_TexCoord;
};

class RawMesh
{
public:
    RawMesh() = default;
    ~RawMesh() = default;

public:
    void Format();

private:
    void CalculateNormals();
    void ProcessVertices();
    void PackVertices();

public:
    std::vector<ethVector3> m_Positions;
    std::vector<ethVector3> m_Normals;
    std::vector<ethVector4> m_Tangents;
    std::vector<ethVector2> m_TexCoords;
    std::vector<uint32_t> m_Indices;
    std::vector<Vertex> m_Vertices;

private:
    std::vector<VertexFormats::VertexFormatStatic> m_PackedData;
};

ETH_NAMESPACE_END

