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
constexpr uint32_t AnimClipVersion = 0;

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
    ostream << m_InverseBindMatrix;
}

void Ether::Graphics::Skeleton::Serialize(OStream& ostream) const
{
    Serializable::Serialize(ostream);

    ostream << NumBones();
    for (uint32_t i = 0; i < m_Bones.size(); ++i)
        m_Bones[i].Serialize(ostream);

    m_BindPose.Serialize(ostream);
}

void Ether::Graphics::SkeletonBone::Deserialize(IStream& istream)
{
    Serializable::Deserialize(istream);

    istream >> m_Name;
    istream >> m_ParentIndex;
    istream >> m_InverseBindMatrix;
}

void Ether::Graphics::SkeletonPose::Serialize(OStream& ostream) const
{
    const uint32_t numBones = m_LocalBoneTransform.size();
    AssertGraphics(m_LocalBoneTransform.size() == m_GlobalBoneTransform.size(), "Bone transforms corrupted");

    ostream << numBones;

    for (uint32_t i = 0; i < numBones; ++i)
    {
        ostream << m_LocalBoneTransform[i];
        ostream << m_GlobalBoneTransform[i];
    }

    ostream << m_GlobalInverseTransform;
}

void Ether::Graphics::SkeletonPose::Deserialize(IStream& istream)
{
    uint32_t numBones;
    istream >> numBones;

    m_LocalBoneTransform.resize(numBones);
    m_GlobalBoneTransform.resize(numBones);

    for (uint32_t i = 0; i < numBones; ++i)
    {
        istream >> m_LocalBoneTransform[i];
        istream >> m_GlobalBoneTransform[i];
    }

    istream >> m_GlobalInverseTransform;
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

Ether::Graphics::Skeleton::Skeleton()
    : Serializable(SkeletonVersion, ETH_CLASS_ID_SKELETON)
{
}

void Ether::Graphics::AnimationClip::BoneKeyframes::Serialize(OStream& ostream) const
{
    ostream << (uint32_t)m_PositionKeyframes.size();
    ostream << (uint32_t)m_RotationKeyframes.size();
    ostream << (uint32_t)m_ScalingKeyframes.size();

    for (uint32_t i = 0; i < m_PositionKeyframes.size(); ++i)
    {
        ostream << m_PositionKeyframes[i].first;
        ostream << m_PositionKeyframes[i].second;
    }

    for (uint32_t i = 0; i < m_RotationKeyframes.size(); ++i)
    {
        ostream << m_RotationKeyframes[i].first;
        ostream << m_RotationKeyframes[i].second;
    }

    for (uint32_t i = 0; i < m_ScalingKeyframes.size(); ++i)
    {
        ostream << m_ScalingKeyframes[i].first;
        ostream << m_ScalingKeyframes[i].second;
    }
}

void Ether::Graphics::AnimationClip::BoneKeyframes::Deserialize(IStream& istream)
{
    uint32_t numPositionKeyframes, numRotationKeyframes, numScaleKeyframes;
    istream >> numPositionKeyframes;
    istream >> numRotationKeyframes;
    istream >> numScaleKeyframes;

    m_PositionKeyframes.resize(numPositionKeyframes);
    m_RotationKeyframes.resize(numRotationKeyframes);
    m_ScalingKeyframes.resize(numScaleKeyframes);

    for (uint32_t i = 0; i < numPositionKeyframes; ++i)
    {
        istream >> m_PositionKeyframes[i].first;
        istream >> m_PositionKeyframes[i].second;
    }

    for (uint32_t i = 0; i < numRotationKeyframes; ++i)
    {
        istream >> m_RotationKeyframes[i].first;
        istream >> m_RotationKeyframes[i].second;
    }
    for (uint32_t i = 0; i < numScaleKeyframes; ++i)
    {
        istream >> m_ScalingKeyframes[i].first;
        istream >> m_ScalingKeyframes[i].second;
    }
}

void Ether::Graphics::AnimationClip::Serialize(OStream& ostream) const
{
    Serializable::Serialize(ostream);

    ostream << m_Name;
    ostream << m_TotalTicks;
    ostream << m_TicksPerSecond;

    ostream << (uint32_t)m_Keyframes.size();
    for (auto iter = m_Keyframes.begin(); iter != m_Keyframes.end(); ++iter)
    {
        ostream << iter->first;
        iter->second.Serialize(ostream);
    }
}

void Ether::Graphics::AnimationClip::Deserialize(IStream& istream)
{
    Serializable::Deserialize(istream);

    istream >> m_Name;
    istream >> m_TotalTicks;
    istream >> m_TicksPerSecond;

    uint32_t numKeyframes;
    istream >> numKeyframes;
    for (uint32_t i = 0; i < numKeyframes; ++i)
    {
        BoneKeyframes keyframes;
        std::string boneName;
        istream >> boneName;
        keyframes.Deserialize(istream);
        m_Keyframes.emplace(boneName, keyframes);
    }
}

template <typename T>
uint32_t GetStartKeyframeIndex(float animTick, const std::vector<std::pair<float, T>>& channel)
{
    for (uint32_t i = 0; i < channel.size() - 1; ++i)
        if (animTick < channel[i + 1].first)
            return i;

    return channel.size() - 1;
}

Ether::ethVector3 Ether::Graphics::AnimationClip::BoneKeyframes::GetInterpolatedPosition(float animTick) const
{
    if (m_PositionKeyframes.size() == 1)
        return m_PositionKeyframes[0].second;

    const uint32_t idx0 = GetStartKeyframeIndex(animTick, m_PositionKeyframes);
    const uint32_t idx1 = idx0 + 1;
    const float t0 = m_PositionKeyframes[idx0].first;
    const float t1 = m_PositionKeyframes[idx1].first;
    const ethVector3 pos0 = m_PositionKeyframes[idx0].second;
    const ethVector3 pos1 = m_PositionKeyframes[idx1].second;

    const float dt = t1 - t0;
    const float a = (animTick - t0) / dt;
    return (pos0 * (1.0f - a)) + (pos1 * a); // Lerp between pos0 and pos1

}

Ether::ethQuaternion Ether::Graphics::AnimationClip::BoneKeyframes::GetInterpolatedRotation(float animTick) const
{
    if (m_RotationKeyframes.size() == 1)
        return m_RotationKeyframes[0].second;

    const uint32_t idx0 = GetStartKeyframeIndex(animTick, m_RotationKeyframes);
    const uint32_t idx1 = idx0 + 1;
    const float t0 = m_RotationKeyframes[idx0].first;
    const float t1 = m_RotationKeyframes[idx1].first;
    const ethQuaternion rot0 = m_RotationKeyframes[idx0].second;
    const ethQuaternion rot1 = m_RotationKeyframes[idx1].second;

    const float dt = t1 - t0;
    const float a = (animTick - t0) / dt;

    return ethQuaternion::Slerp(rot0, rot1, a).Normalized();
}

Ether::ethVector3 Ether::Graphics::AnimationClip::BoneKeyframes::GetInterpolatedScale(float animTick) const
{
    if (m_ScalingKeyframes.size() == 1)
        return m_ScalingKeyframes[0].second;

    const uint32_t idx0 = GetStartKeyframeIndex(animTick, m_ScalingKeyframes);
    const uint32_t idx1 = idx0 + 1;
    const float t0 = m_ScalingKeyframes[idx0].first;
    const float t1 = m_ScalingKeyframes[idx1].first;
    const ethVector3 scale0 = m_ScalingKeyframes[idx0].second;
    const ethVector3 scale1 = m_ScalingKeyframes[idx1].second;

    const float dt = t1 - t0;
    const float a = (animTick - t0) / dt;
    return (scale0 * (1.0f - a)) + (scale1 * a); // Lerp between scale0 and scale1
}

Ether::Graphics::AnimationClip::AnimationClip(
    const std::string& name,
    float totalTicks,
    float ticksPerSecond)
    : Serializable(AnimClipVersion, ETH_CLASS_ID_ANIMATIONCLIP)
    , m_Name(name)
    , m_TotalTicks(totalTicks)
    , m_TicksPerSecond(ticksPerSecond)
{
}

void Ether::Graphics::AnimationClip::AddBoneKeyframes(
    const std::string& boneName,
    const BoneKeyframes& keyframes)
{
    m_Keyframes.emplace(boneName, keyframes);
}

Ether::Graphics::SkeletonPose Ether::Graphics::Skeleton::CalculatePoseFromAnimation(
    const AnimationClip& animation,
    float animTimeTicks) const
{
    SkeletonPose newPose;
    newPose.m_GlobalBoneTransform.resize(NumBones());
    newPose.m_LocalBoneTransform.resize(NumBones());

    // Since bones were populated in DFS fashion, every node is populated after its parent
    for (uint32_t i = 0; i < NumBones(); ++i)
    {
        const SkeletonBone& currentBone = m_Bones[i];
        const bool hasParent = currentBone.m_ParentIndex != InvalidBoneIndex;

        ethMatrix4x4 localTransformation = m_BindPose.m_LocalBoneTransform[i];

        if (animation.HasBoneInfluence(currentBone.m_Name))
        {
            const AnimationClip::BoneKeyframes& keyframes = animation.GetKeyframes(currentBone.m_Name);
            ethVector3 interpolatedPosition = keyframes.GetInterpolatedPosition(animTimeTicks);
            ethVector4 interpolatedRotation = keyframes.GetInterpolatedRotation(animTimeTicks);
            ethVector3 interpolatedScale = keyframes.GetInterpolatedScale(animTimeTicks);

            ethMatrix4x4 translation = Transform::GetTranslationMatrix(interpolatedPosition);
            ethMatrix4x4 rotation = Transform::GetRotationMatrix(interpolatedRotation);
            ethMatrix4x4 scale = Transform::GetScaleMatrix(interpolatedScale);

            localTransformation = translation * rotation * scale;
        }
        
        const ethMatrix4x4 parentTransformation = hasParent ? newPose.m_GlobalBoneTransform[currentBone.m_ParentIndex] : ethMatrix4x4();
        const ethMatrix4x4 globalTransformation = parentTransformation * localTransformation;

        newPose.m_LocalBoneTransform[i] = localTransformation;
        newPose.m_GlobalBoneTransform[i] = globalTransformation;
    }

    return newPose;
}

#if ETH_TOOLMODE
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

uint32_t Ether::Graphics::Skeleton::GetBoneIndex(const std::string& name) const
{
    for (uint32_t i = 0; i < NumBones(); ++i)
        if (m_Bones[i].m_Name == name)
            return i;

    return InvalidBoneIndex;
}
#endif
