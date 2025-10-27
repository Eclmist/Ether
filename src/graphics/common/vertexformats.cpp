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

/*
Semantic names are predefined!!
https://learn.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-semantics

    BINORMAL[n]	Binormal	float4
    BLENDINDICES[n]	Blend indices	uint
    BLENDWEIGHT[n]	Blend weights	float
    COLOR[n]	Diffuse and specular color	float4
    NORMAL[n]	Normal vector	float4
    POSITION[n]	Vertex position in object space.	float4
    POSITIONT	Transformed vertex position.	float4
    PSIZE[n]	Point size	float
    TANGENT[n]	Tangent	float4
    TEXCOORD[n]	Texture coordinates	float4
*/
Ether::Graphics::RhiInputElementDesc Ether::Graphics::VertexFormats::PositionNormalTangentTexcoord::s_InputElementDesc
    [PositionNormalTangentTexcoord_NumElements] = {
        { "POSITION", 0, RhiFormat::R32G32B32Float, 0, 0xffffffff, RhiInputClassification::PerVertexData, 0 },
        { "NORMAL", 0, RhiFormat::R32G32B32Float, 0, 0xffffffff, RhiInputClassification::PerVertexData, 0 },
        { "TANGENT", 0, RhiFormat::R32G32B32Float, 0, 0xffffffff, RhiInputClassification::PerVertexData, 0 },
        { "TEXCOORD", 0, RhiFormat::R32G32Float, 0, 0xffffffff, RhiInputClassification::PerVertexData, 0 },
        { "COLOR", 0, RhiFormat::R32G32B32A32Float, 0, 0xffffffff, RhiInputClassification::PerVertexData, 0 },
    };

void Ether::Graphics::VertexFormats::PositionNormalTangentTexcoord::Serialize(OStream& ostream) const
{
    ostream.WriteBytes(this, sizeof(PositionNormalTangentTexcoord));
}

void Ether::Graphics::VertexFormats::PositionNormalTangentTexcoord::Deserialize(IStream& istream)
{
    istream.ReadBytes(this, sizeof(PositionNormalTangentTexcoord));
}

Ether::Graphics::VertexFormats::PositionNormalTangentTexcoord_Skinned::PositionNormalTangentTexcoord_Skinned()
{
    for (uint32_t i = 0; i < MaxBonesPerVextex; ++i)
        m_BoneIndices[i] = InvalidBoneIndex;
}

void Ether::Graphics::VertexFormats::PositionNormalTangentTexcoord_Skinned::Serialize(OStream& ostream) const
{
    PositionNormalTangentTexcoord::Serialize(ostream);

    for (uint32_t i = 0; i < MaxBonesPerVextex; ++i)
    {
        ostream << m_BoneIndices[i];
        ostream << m_BoneWeights[i];
    }
}

void Ether::Graphics::VertexFormats::PositionNormalTangentTexcoord_Skinned::Deserialize(IStream& istream)
{
    PositionNormalTangentTexcoord::Deserialize(istream);

    for (uint32_t i = 0; i < MaxBonesPerVextex; ++i)
    {
        istream >> m_BoneIndices[i];
        istream >> m_BoneWeights[i];
    }
}

