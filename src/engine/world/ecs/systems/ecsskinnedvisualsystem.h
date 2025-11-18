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

#include "engine/pch.h"
#include "engine/world/ecs/systems/ecssystem.h"
#include "engine/animation/animation.h"
#include "engine/animation/skeleton.h"
#include "graphics/common/visual.h"

namespace Ether::Ecs
{

// TODO: Remove this in favor of storing in a skeleton component
struct SkeletonAnimPairHash
{
    std::size_t operator()(const std::pair<const Skeleton*, const AnimationClip*>& p) const
    {
        // Combine hashes of the two pointers
        std::size_t h1 = std::hash<const Skeleton*>{}(p.first);
        std::size_t h2 = std::hash<const AnimationClip*>{}(p.second);

        // Simple hash combination (you can use boost::hash_combine pattern)
        return h1 ^ (h2 << 1);
    }
};

class EcsSkinnedVisualSystem : public EcsSystem
{
public:
    EcsSkinnedVisualSystem();
    ~EcsSkinnedVisualSystem() override = default;

protected:
    friend class EcsManager;
    void Update() override;

protected:
    SkeletonPose CalculatePoseFromAnimation(const Skeleton& skeleton, const AnimationClip& animation, float animTimeTicks) const;
    void UpdateSkinnedMesh(Graphics::SkinnedMesh& skinnedMesh, const Skeleton& skeleton, const AnimationClip& animationClip);
    bool IsVisualCulled(const Graphics::Visual& visual) const;

private:
    // TODO: Remove this in favor of storing in a skeleton component
    std::unordered_map<std::pair<const Skeleton*, const AnimationClip*>, std::vector<ethMatrix4x4>, SkeletonAnimPairHash> m_ProcessedSkeletalPoses;
};
} // namespace Ether::Ecs
