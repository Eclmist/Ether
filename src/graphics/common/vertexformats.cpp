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

uint32_t Ether::Graphics::VertexFormats::PositionNormalTangentTexcoord::
    s_NumElements = PositionNormalTangentTexcoord_NumElements;

Ether::Graphics::RhiInputElementDesc Ether::Graphics::VertexFormats::PositionNormalTangentTexcoord::s_InputElementDesc
    [PositionNormalTangentTexcoord_NumElements] = {
        { "POSITION", 0, RhiFormat::R32G32B32Float, 0, 0xffffffff, RhiInputClassification::PerVertexData, 0 },
        { "NORMAL", 0, RhiFormat::R32G32B32Float, 0, 0xffffffff, RhiInputClassification::PerVertexData, 0 },
        { "TANGENT", 0, RhiFormat::R32G32B32Float, 0, 0xffffffff, RhiInputClassification::PerVertexData, 0 },
        { "TEXCOORD", 0, RhiFormat::R32G32Float, 0, 0xffffffff, RhiInputClassification::PerVertexData, 0 },
    };

void Ether::Graphics::VertexFormats::PositionNormalTangentTexcoord::Serialize(OStream& ostream) const
{
    ostream.WriteBytes(this, sizeof(PositionNormalTangentTexcoord));
}

void Ether::Graphics::VertexFormats::PositionNormalTangentTexcoord::Deserialize(IStream& istream)
{
    istream.ReadBytes(this, sizeof(PositionNormalTangentTexcoord));
}

