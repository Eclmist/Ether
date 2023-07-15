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

#include "graphics/pch.h"

namespace Ether::Graphics::VertexFormats
{

static constexpr uint32_t PositionNormalTangentTexcoord_NumElements = 4;

class ETH_GRAPHIC_DLL PositionNormalTangentTexcoord
{
public:
    PositionNormalTangentTexcoord() = default;
    ~PositionNormalTangentTexcoord() = default;

public:
    void Serialize(OStream& ostream) const;
    void Deserialize(IStream& istream);

public:
    static RhiInputElementDesc s_InputElementDesc[PositionNormalTangentTexcoord_NumElements];
    static uint32_t s_NumElements;

public:
    ethVector3 m_Position;
    ethVector3 m_Normal;
    ethVector3 m_Tangent;
    ethVector2 m_TexCoord;
};
} // namespace Ether::Graphics::VertexFormats
