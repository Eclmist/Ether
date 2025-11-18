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

#include <execution>

#include "engine/enginecore.h"
#include "engine/world/entity.h"
#include "engine/world/ecs/systems/ecsskinnedvisualsystem.h"
#include "engine/world/ecs/components/ecsvisualcomponent.h"
#include "engine/world/ecs/components/ecscameracomponent.h"
#include "engine/world/ecs/components/ecstransformcomponent.h"

#include "graphics/graphiccore.h"
#include "graphics/common/visualbatch.h"

Ether::Ecs::EcsSkinnedVisualSystem::EcsSkinnedVisualSystem()
{
    m_Signature.set(EcsSkinnedVisualComponent::s_ComponentID);
}

void Ether::Ecs::EcsSkinnedVisualSystem::Update()
{
    ETH_MARKER_EVENT("Skinned Visual System - Update");

    ResourceManager& resources = EngineCore::GetActiveWorld().GetResourceManager();
    Graphics::RenderData& renderData = Graphics::GraphicCore::GetGraphicRenderer().GetThreadedRenderData();
    std::unordered_map<StringID, uint32_t> materialToBatchMap;

    m_ProcessedSkeletalPoses.clear();

    for (EntityID entityID : m_Entities)
    {
        Entity& entity = EngineCore::GetActiveWorld().GetEntity(entityID);
        EcsSkinnedVisualComponent& data = entity.GetComponent<EcsSkinnedVisualComponent>();

        if (!data.m_Enabled)
            continue;

        Graphics::SkinnedVisual gfxVisual;
        Graphics::VisualBatch* gfxVisualBatch;

        if (materialToBatchMap.find(data.m_MaterialGuid) == materialToBatchMap.end())
        {
            renderData.m_VisualBatches.emplace_back();
            materialToBatchMap[data.m_MaterialGuid] = renderData.m_VisualBatches.size() - 1;
            gfxVisualBatch = &renderData.m_VisualBatches[materialToBatchMap.at(data.m_MaterialGuid)];
            if (data.m_MaterialGuid == StringID(""))
            {
                gfxVisualBatch->m_Material = Graphics::GraphicCore::GetGraphicCommon().m_DefaultMaterial.get();
            }
            else
            {
                gfxVisualBatch->m_Material = resources.GetMaterialResource(data.m_MaterialGuid);

                if (gfxVisualBatch->m_Material == nullptr)
                    gfxVisualBatch->m_Material = Graphics::GraphicCore::GetGraphicCommon().m_ErrorMaterial.get();
            }

            gfxVisualBatch->m_Material->SetTransientMaterialIdx(materialToBatchMap.at(data.m_MaterialGuid));
        }
        else
        {
            gfxVisualBatch = &renderData.m_VisualBatches[materialToBatchMap.at(data.m_MaterialGuid)];
        }

        Skeleton* skeleton = resources.GetSkeletonResource(data.m_SkeletonGuid);
        AnimationClip* animClip = resources.GetAnimationClipResource(data.m_AnimationGuid);
        Graphics::SkinnedMesh* skinnedMesh = resources.GetSkinnedMeshResource(data.m_MeshGuid);

        //TODO: Skeleton, Animation, Clips, should really all be their own ECS components.
        // For now, just find the first availble clip that works with the skeleton.

        if (skinnedMesh == nullptr)
            continue;
        if (skeleton == nullptr)
            continue;
        if (animClip == nullptr)
            continue;

        UpdateSkinnedMesh(*skinnedMesh, *skeleton, *animClip);

        gfxVisual.m_Mesh = skinnedMesh;
        gfxVisual.m_Material = gfxVisualBatch->m_Material;
        gfxVisual.m_Culled = false; // TODO: Calculate max AABB for skinned mesh to do proper culling

        if (gfxVisual.m_Material->GetRaytracingVisibility() == Graphics::RaytracingVisibility::Lighting)
            renderData.m_RaytracingVisuals.push_back(gfxVisual);

        renderData.m_Visuals.push_back(gfxVisual);
        renderData.m_RaytracingVisuals.push_back(gfxVisual);
        gfxVisualBatch->m_Visuals.emplace_back(gfxVisual);
    }


    Graphics::GraphicCore::GetRenderThread().EnqueueRenderCommand([&]()
    { 
        Graphics::CommandContext ctx("BLAS Update Context");
        ctx.Reset();

        for (Graphics::Visual visual : renderData.m_Visuals)
        {
            Graphics::SkinnedMesh* skinnedMesh = dynamic_cast<Graphics::SkinnedMesh*>(visual.m_Mesh);

            if (skinnedMesh != nullptr)
            {
                skinnedMesh->UpdateGpuResources(ctx);
            }
        }

        ctx.FinalizeAndExecute();
    });
}

Ether::SkeletonPose Ether::Ecs::EcsSkinnedVisualSystem::CalculatePoseFromAnimation(
    const Skeleton& skeleton,
    const AnimationClip& animation,
    float animTimeTicks) const
{
    ETH_MARKER_EVENT("Skinned Visual System - Pose Computation");

    SkeletonPose newPose;
    newPose.m_GlobalBoneTransforms.resize(skeleton.NumBones());

    // Since bones were populated in DFS fashion, every node is populated after its parent
    for (uint32_t i = 0; i < skeleton.NumBones(); ++i)
    {
        const SkeletonBone& currentBone = skeleton.GetBone(i);
        const bool hasParent = currentBone.m_ParentIndex != InvalidBoneIndex;

        auto positionChannel = dynamic_cast<AnimationClip::AnimationChannel<ethVector3>*>(animation.GetChannel(currentBone.m_Name + "_Position"));
        auto rotationChannel = dynamic_cast<AnimationClip::AnimationChannel<ethQuaternion>*>(animation.GetChannel(currentBone.m_Name + "_Rotation"));
        auto scaleChannel = dynamic_cast<AnimationClip::AnimationChannel<ethVector3>*>(animation.GetChannel(currentBone.m_Name + "_Scale"));

        const bool hasBoneInfluence = positionChannel != nullptr || rotationChannel != nullptr || scaleChannel != nullptr;

        if (!hasBoneInfluence)
            continue;

        ethMatrix4x4 translation, rotation, scale;

        if (positionChannel != nullptr)
        {
            ethVector3 interpolatedPosition = positionChannel->GetInterpolatedValue(
                animTimeTicks,
                [](const ethVector3& a, const ethVector3& b, float t) { return (a * (1 - t)) + (b * t); });

            translation = Transform::GetTranslationMatrix(interpolatedPosition);
        }

        if (rotationChannel != nullptr)
        {
            ethQuaternion interpolatedRotation = rotationChannel->GetInterpolatedValue(
                animTimeTicks,
                [](const ethQuaternion& a, const ethQuaternion& b, float t) { return ethQuaternion::Slerp(a, b, t).Normalized(); });

            rotation = Transform::GetRotationMatrix(interpolatedRotation);
        }

        if (scaleChannel != nullptr)
        {
            ethVector3 interpolatedScale = scaleChannel->GetInterpolatedValue(
                animTimeTicks,
                [](const ethVector3& a, const ethVector3& b, float t) { return (a * (1 - t)) + (b * t); });

            scale = Transform::GetScaleMatrix(interpolatedScale);
        }

        const ethMatrix4x4 localTransformation = translation * rotation * scale;
        const ethMatrix4x4 parentTransformation = hasParent ? newPose.m_GlobalBoneTransforms[currentBone.m_ParentIndex]
                                                            : ethMatrix4x4();
        const ethMatrix4x4 globalTransformation = parentTransformation * localTransformation;

        newPose.m_GlobalBoneTransforms[i] = globalTransformation;
    }

    return newPose;
}

void Ether::Ecs::EcsSkinnedVisualSystem::UpdateSkinnedMesh(
    Graphics::SkinnedMesh& skinnedMesh,
    const Skeleton& skeleton,
    const AnimationClip& animationClip)
{
    ETH_MARKER_EVENT("CPU Skinning");

    // --- CPU Skinning --- //
    // Loop animation time
    float animationTime = Time::GetTimeSinceStartup();

    // RTCamp11 Hack (TODO)
    const float ticksPerSecond = animationClip.GetTicksPerSecond();
    const float totalTicks = animationClip.GetTotalTicks();
    const float timeInSeconds = animationTime / 1000.0f;
    const float loopedTimeInTicks = std::fmod(timeInSeconds * ticksPerSecond, totalTicks);

    std::vector<ethMatrix4x4> boneMatrices(skeleton.NumBones());

    std::pair<const Skeleton*, const AnimationClip*> skeletalAnimation = std::make_pair(&skeleton, &animationClip);

    if (m_ProcessedSkeletalPoses.contains(skeletalAnimation))
    {
        boneMatrices = m_ProcessedSkeletalPoses.at(skeletalAnimation);
    }
    else
    {
        ETH_MARKER_EVENT("CPU Skinning - Bone Matrices");

        const SkeletonPose pose = CalculatePoseFromAnimation(skeleton, animationClip, loopedTimeInTicks);

        for (uint32_t b = 0; b < skeleton.NumBones(); ++b)
            boneMatrices[b] = pose.m_GlobalBoneTransforms[b] * skeleton.GetBone(b).m_InverseBindMatrix;

        m_ProcessedSkeletalPoses.emplace(skeletalAnimation, boneMatrices);
    }
    
    {
        ETH_MARKER_EVENT("CPU Skinning - Vertex Arithmetics");

        std::vector<Graphics::VertexFormats::SkinnedVertexFormat>& skinningVertices = skinnedMesh.GetSkinningVertices();
        std::vector<Graphics::VertexFormats::BaseVertexFormat> stagingVertices = skinnedMesh.GetStagingVertices();
        const uint32_t numVertices = skinningVertices.size();

        std::for_each(
            std::execution::par,
            std::begin(skinningVertices),
            std::begin(skinningVertices) + numVertices,
            [&](const Graphics::VertexFormats::SkinnedVertexFormat& src)
            {
                const uint32_t i = &src - &skinningVertices[0];
                ethVector4 skinnedPos(0, 0, 0, 0);
                ethVector4 skinnedNormal(0, 0, 0, 0);

                for (uint32_t j = 0; j < MaxBonesPerVextex; ++j)
                {
                    const uint32_t boneIndex = skinningVertices[i].m_BoneIndices[j];
                    const float weight = skinningVertices[i].m_BoneWeights[j];
                    if (weight <= 0.0f || boneIndex == InvalidBoneIndex)
                        continue;

                    const ethMatrix4x4& finalBoneMatrix = boneMatrices[boneIndex];
                    const auto& pos = skinningVertices[i].m_Attributes.m_Position;
                    const auto& normal = skinningVertices[i].m_Attributes.m_Normal;
                    skinnedPos += (finalBoneMatrix * ethVector4(pos.x, pos.y, pos.z, 1.0f)) * weight;
                    skinnedNormal += (finalBoneMatrix * ethVector4(normal.x, normal.y, normal.z, 0.0f)) * weight;
                }

                stagingVertices[i].m_Attributes.m_PrevPosition = stagingVertices[i].m_Attributes.m_Position;
                stagingVertices[i].m_Attributes.m_Position = skinnedPos.Resize<3>();
                stagingVertices[i].m_Attributes.m_Normal = skinnedNormal.Resize<3>().Normalized();
            });

        Graphics::GraphicCore::GetRenderThread().EnqueueRenderCommand(
            [stagingVertices = std::move(stagingVertices), &skinnedMesh]() mutable {
                skinnedMesh.SetStagingVertices(std::move(stagingVertices));
        });
    }
}

bool Ether::Ecs::EcsSkinnedVisualSystem::IsVisualCulled(const Graphics::Visual& visual) const
{
    Entity* camera = EngineCore::GetActiveWorld().GetMainCamera();
    if (camera == nullptr)
        return false;

    Graphics::RenderData& renderData = Graphics::GraphicCore::GetGraphicRenderer().GetThreadedRenderData();

    Aabb visualAabb = visual.m_Mesh->GetBoundingBox();

    ethMatrix4x4 viewProjectionMatrix = renderData.m_ProjectionMatrix * renderData.m_ViewMatrix;

    ethVector4 planes[6];
    planes[0] = ethVector4(viewProjectionMatrix.m_Data2D[3]) - ethVector4(viewProjectionMatrix.m_Data2D[2]);
    planes[1] = ethVector4(viewProjectionMatrix.m_Data2D[2]);
    planes[2] = ethVector4(viewProjectionMatrix.m_Data2D[3]) + ethVector4(viewProjectionMatrix.m_Data2D[0]);
    planes[3] = ethVector4(viewProjectionMatrix.m_Data2D[3]) - ethVector4(viewProjectionMatrix.m_Data2D[0]);
    planes[4] = ethVector4(viewProjectionMatrix.m_Data2D[3]) - ethVector4(viewProjectionMatrix.m_Data2D[1]);
    planes[5] = ethVector4(viewProjectionMatrix.m_Data2D[3]) + ethVector4(viewProjectionMatrix.m_Data2D[1]);

    for (uint32_t i = 0; i < 6; ++i)
    {
        ethVector3 normal = planes[i].Resize<3>();
        planes[i] /= normal.Magnitude();
    }

    for (uint32_t i = 0; i < 6; ++i)
    {
        ethVector4 vert;
        vert.x = planes[i].x > 0 ? visualAabb.m_Max.x : visualAabb.m_Min.x;
        vert.y = planes[i].y > 0 ? visualAabb.m_Max.y : visualAabb.m_Min.y;
        vert.z = planes[i].z > 0 ? visualAabb.m_Max.z : visualAabb.m_Min.z;
        vert.w = 1.0f;

        if (ethVector4::Dot(vert, planes[i]) < 0.0f)
            return false;
    }
    return true;
}

