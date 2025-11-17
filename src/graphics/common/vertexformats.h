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

#pragma once

#include "graphics/pch.h"
#include "graphics/shaders/common/vertexcommon.h"

namespace Ether::Graphics::VertexFormats
{
static constexpr uint32_t BaseVertexFormat_NumElements = 6;
static constexpr uint32_t MaxBonesPerVertex = 4;
static constexpr uint32_t InvalidBoneIndex = -1;

class ETH_GRAPHIC_DLL BaseVertexFormat
{
public:
    BaseVertexFormat() = default;
    ~BaseVertexFormat() = default;

public:
    void Serialize(OStream& ostream) const;
    void Deserialize(IStream& istream);

public:
    static RhiInputElementDesc s_InputElementDesc[BaseVertexFormat_NumElements];
    static uint32_t s_NumElements;

public:
    Shader::MeshVertex m_Attributes;
};

class ETH_GRAPHIC_DLL SkinnedVertexFormat : public BaseVertexFormat
{
public:
    SkinnedVertexFormat();
    ~SkinnedVertexFormat() = default;

public:
    void Serialize(OStream& ostream) const;
    void Deserialize(IStream& istream);

public:
    // CPU side data for skinning
    uint32_t m_BoneIndices[MaxBonesPerVertex]; // 4 x u8 packed into u32 (or uint32[4])
    float m_BoneWeights[MaxBonesPerVertex];                  // 4 floats
};
} // namespace Ether::Graphics::VertexFormats 
