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

#define ETH_CLASS_ID_SKELETON "Graphics::Skeleton"
#define ETH_CLASS_ID_SKELETONBONE "Graphics::SkeletonBone"
#define ETH_CLASS_ID_SKELETONANIMATIONCLIP "Graphics::SkeletonAnimationClip"

namespace Ether::Graphics
{
static constexpr uint32_t MaxBonesPerVextex = 4;
static constexpr uint32_t InvalidBoneIndex = -1;

class ETH_GRAPHIC_DLL SkeletonBone : public Serializable
{
public:
    SkeletonBone();
    SkeletonBone(const std::string& boneName, uint32_t parentIndex, const ethMatrix4x4& inverseBindMatrix);
    ~SkeletonBone() = default;

public:
    void Serialize(OStream& ostream) const override;
    void Deserialize(IStream& istream) override;

public:
    std::string m_Name;
    uint32_t m_ParentIndex;
    ethMatrix4x4 m_InverseBindMatrix;
};

struct ETH_GRAPHIC_DLL SkeletonPose
{
    void Serialize(OStream& ostream) const;
    void Deserialize(IStream& istream);

    std::vector<ethMatrix4x4> m_LocalBoneTransform;
    std::vector<ethMatrix4x4> m_GlobalBoneTransform;
};

class ETH_GRAPHIC_DLL Skeleton : public Serializable
{
public:
    Skeleton();
    ~Skeleton() override = default;

public:
    void Serialize(OStream& ostream) const override;
    void Deserialize(IStream& istream) override;

public:
    inline uint32_t NumBones() const { return m_Bones.size(); }
    inline const SkeletonBone& GetBone(uint32_t index) const { return m_Bones[index]; }
    inline const SkeletonPose& GetBindPose() const { return m_BindPose; }

    inline void AddBone(const SkeletonBone& bone) { m_Bones.push_back(bone); }
    inline void SetBindPose(const SkeletonPose bindPose) { m_BindPose = bindPose; }

public:
    void DebugPrint(uint32_t parentIndex = UINT32_MAX, const std::string& prefix = "", bool isLast = true) const;

private:
    std::vector<SkeletonBone> m_Bones;
    SkeletonPose m_BindPose;
};

class ETH_GRAPHIC_DLL SkeletonAnimationClip : public Serializable
{
public:
    SkeletonAnimationClip(
        const std::string& name,
        float duration,
        const std::vector<SkeletonPose>& keyframes,
        const std::vector<float>& times);
    ~SkeletonAnimationClip() override = default;

public:
    inline const std::string& GetName() const { return m_Name; }
    inline const float GetAnimDuration() const { return m_AnimDuration; }
    inline const std::vector<SkeletonPose>& GetKeyframes() const { return m_Keyframes; }
    inline const std::vector<float>& GetKeyframeTimes() const { return m_KeyframeTimes; }

public:
    void Serialize(OStream& ostream) const override;
    void Deserialize(IStream& istream) override;

private:
    std::string m_Name;
    float m_AnimDuration;

    std::vector<SkeletonPose> m_Keyframes;
    std::vector<float> m_KeyframeTimes;
};

} // namespace Ether::Graphics
