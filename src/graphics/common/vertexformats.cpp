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

#include "graphics/common/vertexformats.h"

Ether::Graphics::RhiInputElementDesc
    Ether::Graphics::VertexFormats::PositionNormalTangentBitangentTexcoord::s_InputElementDesc[5] = {
        { "POSITION", 0, RhiFormat::R32G32B32Float, 0, 0xffffffff, RhiInputClassification::PerVertexData, 0 },
        { "NORMAL", 0, RhiFormat::R32G32B32Float, 0, 0xffffffff, RhiInputClassification::PerVertexData, 0 },
        { "TANGENT", 0, RhiFormat::R32G32B32Float, 0, 0xffffffff, RhiInputClassification::PerVertexData, 0 },
        { "BITANGENT", 0, RhiFormat::R32G32B32Float, 0, 0xffffffff, RhiInputClassification::PerVertexData, 0 },
        { "TEXCOORD", 0, RhiFormat::R32G32Float, 0, 0xffffffff, RhiInputClassification::PerVertexData, 0 },
    };

uint32_t Ether::Graphics::VertexFormats::PositionNormalTangentBitangentTexcoord::s_NumElements = 5;

void Ether::Graphics::VertexFormats::PositionNormalTangentBitangentTexcoord::Serialize(OStream& ostream) const
{
    ostream << m_Position;
    ostream << m_Normal;
    ostream << m_Tangent;
    ostream << m_BiTangent;
    ostream << m_TexCoord;
}

void Ether::Graphics::VertexFormats::PositionNormalTangentBitangentTexcoord::Deserialize(IStream& istream)
{
    istream >> m_Position;
    istream >> m_Normal;
    istream >> m_Tangent;
    istream >> m_BiTangent;
    istream >> m_TexCoord;
}

