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
class ETH_GRAPHIC_DLL PositionNormalTangentTexCoord
{
public:
    PositionNormalTangentTexCoord() = default;
    ~PositionNormalTangentTexCoord() = default;

    void Serialize(OStream& ostream) const;
    void Deserialize(IStream& istream);

    ethVector3 m_Position;
    ethVector3 m_Normal;
    ethVector3 m_Tangent;
    ethVector3 m_BiTangent;
    ethVector2 m_TexCoord;
};
} // namespace Ether::Graphics::VertexFormats
