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

#include "graphics/resources/skeleton.h"

constexpr uint32_t SkeletonVersion = 0;
constexpr uint32_t SkeletonBoneVersion = 0;

Ether::Graphics::SkeletonBone::SkeletonBone()
    : Serializable(SkeletonBoneVersion, ETH_CLASS_ID_SKELETONBONE)
    , m_Name("InvalidBone")
    , m_ParentIndex(InvalidBoneIndex)
    , m_InverseBindMatrix(ethMatrix4x4{})
{
}

Ether::Graphics::SkeletonBone::SkeletonBone(
    const std::string& boneName,
    uint32_t parentIndex,
    const ethMatrix4x4& inverseBindMatrix)
    : Serializable(SkeletonBoneVersion, ETH_CLASS_ID_SKELETONBONE)
    , m_Name(boneName)
    , m_ParentIndex(parentIndex)
    , m_InverseBindMatrix(inverseBindMatrix)
{
}

void Ether::Graphics::SkeletonBone::Serialize(OStream& ostream) const
{
    Serializable::Serialize(ostream);

    ostream << m_Name;
    ostream << m_ParentIndex;

    // Lazy to add a matrix overload for now (todo)
    for (uint32_t i = 0; i < 16; ++i)
        ostream << m_InverseBindMatrix.m_Data[i];
}

void Ether::Graphics::SkeletonBone::Deserialize(IStream& istream)
{
    Serializable::Deserialize(istream);

    istream >> m_Name;
    istream >> m_ParentIndex;

    // Lazy to add a matrix overload for now (todo)
    for (uint32_t i = 0; i < 16; ++i)
        istream >> m_InverseBindMatrix.m_Data[i];
}

Ether::Graphics::Skeleton::Skeleton()
    : Serializable(SkeletonVersion, ETH_CLASS_ID_SKELETON)
{
}

void Ether::Graphics::Skeleton::Serialize(OStream& ostream) const
{
    Serializable::Serialize(ostream);

    ostream << NumBones();
    for (uint32_t i = 0; i < m_Bones.size(); ++i)
        m_Bones[i].Serialize(ostream);
}

void Ether::Graphics::Skeleton::Deserialize(IStream& istream)
{
    Serializable::Deserialize(istream);

    uint32_t numBones = 0;
    istream >> numBones;

    m_Bones.resize(numBones);
    for (uint32_t i = 0; i < numBones; ++i)
    {
        m_Bones[i].Deserialize(istream);
    }
}

