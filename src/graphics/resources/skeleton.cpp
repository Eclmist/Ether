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

    // Lazy to add a matrix overload for now (todo)
    for (uint32_t i = 0; i < 16; ++i)
        ostream << m_InverseBindMatrix.m_Data[i];
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

    // Lazy to add a matrix overload for now (todo)
    for (uint32_t i = 0; i < 16; ++i)
        istream >> m_InverseBindMatrix.m_Data[i];
}

void Ether::Graphics::SkeletonPose::Serialize(OStream& ostream) const
{
    const uint32_t numBones = m_LocalBoneTransform.size();
    AssertGraphics(m_LocalBoneTransform.size() == m_GlobalBoneTransform.size(), "Bone transforms corrupted");

    ostream << numBones;

    // Lazy to add a matrix overload for now (todo)
    for (uint32_t i = 0; i < numBones; ++i)
    {
        for (uint32_t j = 0; j < 16; ++j)
            ostream << m_LocalBoneTransform[i].m_Data[j];

        for (uint32_t j = 0; j < 16; ++j)
            ostream << m_GlobalBoneTransform[i].m_Data[j];
    }

    for (uint32_t j = 0; j < 16; ++j)
        ostream << m_GlobalInverseTransform.m_Data[j];
}

void Ether::Graphics::SkeletonPose::Deserialize(IStream& istream)
{
    uint32_t numBones;
    istream >> numBones;

    m_LocalBoneTransform.resize(numBones);
    m_GlobalBoneTransform.resize(numBones);

    // Lazy to add a matrix overload for now (todo)
    for (uint32_t i = 0; i < numBones; ++i)
    {
        for (uint32_t j = 0; j < 16; ++j)
            istream >> m_LocalBoneTransform[i].m_Data[j];

        for (uint32_t j = 0; j < 16; ++j)
            istream >> m_GlobalBoneTransform[i].m_Data[j];
    }

    for (uint32_t j = 0; j < 16; ++j)
        istream >> m_GlobalInverseTransform.m_Data[j];
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

// BIBG BIG BIG BIG HACK!!!
// SMath does not have quaternion support yet, so we're gonna do it here!! (rtcamp)
Ether::ethVector4 QuaternionSlerp(const Ether::ethVector4& q1, const Ether::ethVector4& q2, float t)
{
    float cosTheta = q1.x * q2.x + q1.y * q2.y + q1.z * q2.z + q1.w * q2.w;

    Ether::ethVector4 q2Copy = q2;
    if (cosTheta < 0.0f)
    {
        cosTheta = -cosTheta;
        q2Copy.x = -q2Copy.x;
        q2Copy.y = -q2Copy.y;
        q2Copy.z = -q2Copy.z;
        q2Copy.w = -q2Copy.w;
    }

    const float EPSILON = 1e-6f;
    if (cosTheta > 1.0f - EPSILON)
    {
        // Lerp
        Ether::ethVector4 result;
        result.x = q1.x + t * (q2Copy.x - q1.x);
        result.y = q1.y + t * (q2Copy.y - q1.y);
        result.z = q1.z + t * (q2Copy.z - q1.z);
        result.w = q1.w + t * (q2Copy.w - q1.w);

        // Normalize
        float len = std::sqrt(result.x * result.x + result.y * result.y + result.z * result.z + result.w * result.w);
        result.x /= len;
        result.y /= len;
        result.z /= len;
        result.w /= len;
        return result;
    }

    // Standard slerp
    float angle = std::acos(cosTheta);
    float sinAngle = std::sqrt(1.0f - cosTheta * cosTheta);

    float a = std::sin((1.0f - t) * angle) / sinAngle;
    float b = std::sin(t * angle) / sinAngle;

    Ether::ethVector4 result;
    result.x = a * q1.x + b * q2Copy.x;
    result.y = a * q1.y + b * q2Copy.y;
    result.z = a * q1.z + b * q2Copy.z;
    result.w = a * q1.w + b * q2Copy.w;
    return result;
}

Ether::ethVector4 QuaternionNormalize(const Ether::ethVector4& q)
{
    float len = std::sqrt(q.x * q.x + q.y * q.y + q.z * q.z + q.w * q.w);
    if (len > 0.0f)
    {
        float invLen = 1.0f / len;
        return { q.x * invLen, q.y * invLen, q.z * invLen, q.w * invLen };
    }
    // Return identity quaternion if zero length
    return { 0.0f, 0.0f, 0.0f, 1.0f };
}

Ether::ethMatrix4x4 QuaternionToMatrixRowMajor(const Ether::ethVector4& q)
{
    float x = q.x, y = q.y, z = q.z, w = q.w;

    float x2 = x + x;
    float y2 = y + y;
    float z2 = z + z;

    float xx = x * x2;
    float yy = y * y2;
    float zz = z * z2;
    float xy = x * y2;
    float xz = x * z2;
    float yz = y * z2;
    float wx = w * x2;
    float wy = w * y2;
    float wz = w * z2;

    // row-major
    Ether::ethMatrix4x4 out;
    out.m_Data2D[0][0] = 1.0f - (yy + zz);
    out.m_Data2D[0][1] = xy - wz;
    out.m_Data2D[0][2] = xz + wy;
    out.m_Data2D[0][3] = 0.0f;

    out.m_Data2D[1][0] = xy + wz;
    out.m_Data2D[1][1] = 1.0f - (xx + zz);
    out.m_Data2D[1][2] = yz - wx;
    out.m_Data2D[1][3] = 0.0f;

    out.m_Data2D[2][0] = xz - wy;
    out.m_Data2D[2][1] = yz + wx;
    out.m_Data2D[2][2] = 1.0f - (xx + yy);
    out.m_Data2D[2][3] = 0.0f;

    out.m_Data2D[3][0] = 0.0f;
    out.m_Data2D[3][1] = 0.0f;
    out.m_Data2D[3][2] = 0.0f;
    out.m_Data2D[3][3] = 1.0f;

    return out;
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

void Ether::Graphics::AnimationClip::Serialize(OStream& ostream) const
{
    Serializable::Serialize(ostream);

    ostream << m_Name;
    ostream << m_AnimDuration;

    ostream << (uint32_t)m_Keyframes.size();
    for (auto iter = m_Keyframes.begin(); iter != m_Keyframes.end(); ++iter)
    {
        ostream << iter->first;
        iter->second.Serialize(ostream);
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

void Ether::Graphics::AnimationClip::Deserialize(IStream& istream)
{
    Serializable::Deserialize(istream);

    istream >> m_Name;
    istream >> m_AnimDuration;

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

Ether::ethVector4 Ether::Graphics::AnimationClip::BoneKeyframes::GetInterpolatedRotation(float animTick) const
{
    if (m_RotationKeyframes.size() == 1)
        return m_RotationKeyframes[0].second;

    const uint32_t idx0 = GetStartKeyframeIndex(animTick, m_RotationKeyframes);
    const uint32_t idx1 = idx0 + 1;
    const float t0 = m_RotationKeyframes[idx0].first;
    const float t1 = m_RotationKeyframes[idx1].first;
    const ethVector4 rot0 = m_RotationKeyframes[idx0].second;
    const ethVector4 rot1 = m_RotationKeyframes[idx1].second;

    const float dt = t1 - t0;
    const float a = (animTick - t0) / dt;
    ethVector4 rotOut = QuaternionSlerp(rot0, rot1, a);
    rotOut = QuaternionNormalize(rotOut);

    return rotOut;
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
    float duration)
    : Serializable(AnimClipVersion, ETH_CLASS_ID_ANIMATIONCLIP)
    , m_Name(name)
    , m_AnimDuration(duration)
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
            ethMatrix4x4 rotation = QuaternionToMatrixRowMajor(interpolatedRotation);
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

