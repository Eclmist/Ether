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
#include <unordered_set>

#define ETH_CLASS_ID_SKELETON "Graphics::Skeleton"
#define ETH_CLASS_ID_SKELETONBONE "Graphics::SkeletonBone"
#define ETH_CLASS_ID_ANIMATIONCLIP "Graphics::AnimationClip"

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
    ethMatrix4x4 m_GlobalInverseTransform;
};

class ETH_GRAPHIC_DLL AnimationClip : public Serializable
{
public:
    struct BoneKeyframes
    {
        void Serialize(OStream& ostream) const;
        void Deserialize(IStream& istream);

        ethVector3 GetInterpolatedPosition(float animTick) const;
        ethVector4 GetInterpolatedRotation(float animTick) const;
        ethVector3 GetInterpolatedScale(float animTick) const;

        std::vector<std::pair<float, ethVector3>> m_PositionKeyframes;
        std::vector<std::pair<float, ethVector4>> m_RotationKeyframes;
        std::vector<std::pair<float, ethVector3>> m_ScalingKeyframes;
    };

public:
    AnimationClip(const std::string& name = "Unnamed Animation Clip", float totalTicks = 0.0f, float ticksPerSecond = 30.0f);
    ~AnimationClip() override = default;

public:
    void Serialize(OStream& ostream) const override;
    void Deserialize(IStream& istream) override;

public:
    inline const std::string& GetName() const { return m_Name; }
    inline const float GetTotalTicks() const { return m_TotalTicks; }
    inline const float GetTicksPerSecond() const { return m_TicksPerSecond; }

    inline bool HasBoneInfluence(const std::string& boneName) const { return m_Keyframes.find(boneName) != m_Keyframes.end();}
    inline const BoneKeyframes& GetKeyframes(const std::string& boneName) const { return m_Keyframes.at(boneName); }

public:
    void AddBoneKeyframes(const std::string& boneName, const BoneKeyframes& keyframes);

private:
    std::string m_Name;
    float m_TotalTicks;
    float m_TicksPerSecond;
    std::unordered_map<std::string, BoneKeyframes> m_Keyframes;
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
    inline void SetCurrentPose(const SkeletonPose pose) { m_CurrentPose = pose; }

public:
    //TOOD: make static? This doesn't feel like the right place to put this
    SkeletonPose CalculatePoseFromAnimation(const AnimationClip& animation, float animTimeTicks) const;

public:
#if ETH_TOOLMODE
    uint32_t GetBoneIndex(const std::string& name) const;
    void DebugPrint(uint32_t parentIndex = UINT32_MAX, const std::string& prefix = "", bool isLast = true) const;
#endif

private:
    std::vector<SkeletonBone> m_Bones;
    SkeletonPose m_BindPose;
    SkeletonPose m_CurrentPose;
};

} // namespace Ether::Graphics
