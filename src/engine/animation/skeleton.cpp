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

#include <functional>
#include "engine/animation/skeleton.h"

constexpr uint32_t SkeletonVersion = 0;

Ether::SkeletonBone::SkeletonBone()
    : m_Name("InvalidBone")
    , m_ParentIndex(InvalidBoneIndex)
    , m_InverseBindMatrix(ethMatrix4x4{})
{
}

Ether::SkeletonBone::SkeletonBone(
    const std::string& boneName,
    uint32_t parentIndex,
    const ethMatrix4x4& inverseBindMatrix)
    : m_Name(boneName)
    , m_ParentIndex(parentIndex)
    , m_InverseBindMatrix(inverseBindMatrix)
{
}

void Ether::SkeletonBone::Serialize(OStream& ostream) const
{
    ostream << m_Name;
    ostream << m_ParentIndex;
    ostream << m_InverseBindMatrix;
}

void Ether::SkeletonBone::Deserialize(IStream& istream)
{
    istream >> m_Name;
    istream >> m_ParentIndex;
    istream >> m_InverseBindMatrix;
}

void Ether::SkeletonPose::Serialize(OStream& ostream) const
{
    const uint32_t numBones = m_GlobalBoneTransforms.size();
    ostream << numBones;
    for (uint32_t i = 0; i < numBones; ++i)
    {
        ostream << m_GlobalBoneTransforms[i];
    }
}

void Ether::SkeletonPose::Deserialize(IStream& istream)
{
    uint32_t numBones;
    istream >> numBones;

    m_GlobalBoneTransforms.resize(numBones);

    for (uint32_t i = 0; i < numBones; ++i)
    {
        istream >> m_GlobalBoneTransforms[i];
    }
}

Ether::Skeleton::Skeleton()
    : Serializable(SkeletonVersion, ETH_CLASS_ID_SKELETON)
{
}

void Ether::Skeleton::Serialize(OStream& ostream) const
{
    Serializable::Serialize(ostream);

    ostream << NumBones();
    for (uint32_t i = 0; i < m_Bones.size(); ++i)
        m_Bones[i].Serialize(ostream);
}

void Ether::Skeleton::Deserialize(IStream& istream)
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

#if ETH_TOOLMODE
uint32_t Ether::Skeleton::GetBoneIndex(const std::string& name) const
{
    for (uint32_t i = 0; i < NumBones(); ++i)
        if (m_Bones[i].m_Name == name)
            return i;

    return InvalidBoneIndex;
}

void Ether::Skeleton::DebugPrint(uint32_t parentIndex, const std::string& prefix, bool isLast) const
{
    // Count children for this parent
    std::vector<uint32_t> children;
    for (uint32_t i = 0; i < NumBones(); ++i)
    {
        if (GetBone(i).m_ParentIndex == parentIndex)
            children.push_back(i);
    }

    for (uint32_t i = 0; i < children.size(); ++i)
    {
        uint32_t childBoneIndex = children[i];
        bool childIsLast = (i == children.size() - 1);

        std::string connector = childIsLast ? "„¤„Ÿ " : "„¥„Ÿ ";
        std::string line = prefix + connector + GetBone(childBoneIndex).m_Name;
        LogInfo("%s", line.c_str());

        // Prefix for next level
        std::string childPrefix = prefix + (childIsLast ? "   " : "„   ");
        DebugPrint(childBoneIndex, childPrefix, childIsLast);
    }
}
#endif
