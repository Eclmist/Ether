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

#include <functional>
#include "graphics/resources/skeleton.h"

constexpr uint32_t SkeletonVersion = 0;
constexpr uint32_t SkeletonBoneVersion = 0;
constexpr uint32_t SkeletonAnimationVersion = 0;

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

void Ether::Graphics::SkeletonPose::Serialize(OStream& ostream) const
{
    const uint32_t numBones = m_LocalBoneTransform.size();
    AssertGraphics(m_LocalBoneTransform.size() == m_GlobalBoneTransform.size(), "Bone transforms corrupted");

    ostream << numBones;

    for (uint32_t i = 0; i < numBones; ++i)
    {
        for (uint32_t j = 0; j < 16; ++j)
            ostream << m_LocalBoneTransform[i].m_Data[j];

        for (uint32_t j = 0; j < 16; ++j)
            ostream << m_GlobalBoneTransform[i].m_Data[j];
    }
}

void Ether::Graphics::SkeletonPose::Deserialize(IStream& istream)
{
    uint32_t numBones;
    istream >> numBones;

    m_LocalBoneTransform.resize(numBones);
    m_GlobalBoneTransform.resize(numBones);

    for (uint32_t i = 0; i < numBones; ++i)
    {
        for (uint32_t j = 0; j < 16; ++j)
            istream >> m_LocalBoneTransform[i].m_Data[j];

        for (uint32_t j = 0; j < 16; ++j)
            istream >> m_GlobalBoneTransform[i].m_Data[j];
    }
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

    m_BindPose.Serialize(ostream);
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

    m_BindPose.Deserialize(istream);
}

void Ether::Graphics::Skeleton::DebugPrint(uint32_t parentIndex, const std::string& prefix, bool isLast) const
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

Ether::Graphics::SkeletonAnimationClip::SkeletonAnimationClip(
    const std::string& name,
    float duration,
    const std::vector<SkeletonPose>& keyframes,
    const std::vector<float>& times)
    : Serializable(SkeletonAnimationVersion, ETH_CLASS_ID_SKELETONANIMATIONCLIP)
    , m_Name(name)
    , m_AnimDuration(duration)
    , m_Keyframes(keyframes)
    , m_KeyframeTimes(times)
{
}

void Ether::Graphics::SkeletonAnimationClip::Serialize(OStream& ostream) const
{
    ostream << m_Name;
    ostream << m_AnimDuration;

    const uint32_t numKeyframes = m_Keyframes.size();
    ostream << numKeyframes;

    for (uint32_t i = 0; i < numKeyframes; ++i)
        m_Keyframes[i].Serialize(ostream);

    for (uint32_t i = 0; i < numKeyframes; ++i)
        ostream << m_KeyframeTimes[i];
}

void Ether::Graphics::SkeletonAnimationClip::Deserialize(IStream& istream)
{
    istream >> m_Name;
    istream >> m_AnimDuration;

    uint32_t numKeyframes;
    istream >> numKeyframes;

    m_Keyframes.resize(numKeyframes);
    m_KeyframeTimes.resize(numKeyframes);

    for (uint32_t i = 0; i < numKeyframes; ++i)
    {
        m_Keyframes[i].Deserialize(istream);
        istream >> m_KeyframeTimes[i];
    }
}

