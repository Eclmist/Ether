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

#include <execution>
#include "graphics/resources/skinnedmesh.h"
#include "graphics/graphiccore.h"

constexpr uint32_t SkinnedMeshVersion = 1;

Ether::Graphics::SkinnedMesh::SkinnedMesh()
    : Mesh(SkinnedMeshVersion, ETH_CLASS_ID_SKINNEDMESH)
{
}

void Ether::Graphics::SkinnedMesh::Serialize(OStream& ostream) const
{
    Mesh::Serialize(ostream);

    ostream << m_NumVertices;
    for (int i = 0; i < m_PackedVertices.size(); ++i)
        m_PackedVertices[i].Serialize(ostream);

    ostream << m_SkeletonGuid;
}

void Ether::Graphics::SkinnedMesh::Deserialize(IStream& istream)
{
    Mesh::Deserialize(istream);

    istream >> m_NumVertices;
    AssertGraphics(m_NumVertices <= MaxVerticesPerMesh, "Num vertices exceeds limit");

    m_PackedVertices.resize(m_NumVertices);
    for (int i = 0; i < m_PackedVertices.size(); ++i)
        m_PackedVertices[i].Deserialize(istream);

    istream >> m_SkeletonGuid;
}

void Ether::Graphics::SkinnedMesh::CreateGpuResources(CommandContext& ctx)
{
    m_GPUCompatiblePackedVertices.resize(m_PackedVertices.size());
    for (uint32_t i = 0; i < m_PackedVertices.size(); ++i)
    {
        m_GPUCompatiblePackedVertices[i].m_Attributes = m_PackedVertices[i].m_Attributes;
    }

    CreateVertexBuffer(ctx);
    CreateIndexBuffer(ctx);
    CreateAccelerationStructure(ctx, true /* allow update */);

    // RtCamp11 hacks (TODO)
    CreateStagingVertexBuffer();
    m_PackedVerticesOriginal = m_PackedVertices;
}

void Ether::Graphics::SkinnedMesh::ComputeBoundingBox()
{
    m_BoundingBox.m_Min = 9999999;
    m_BoundingBox.m_Max = -9999999;

    for (auto& vertex : m_PackedVertices)
    {
        m_BoundingBox.m_Min.x = std::min(m_BoundingBox.m_Min.x, vertex.m_Attributes.m_Position.x);
        m_BoundingBox.m_Min.y = std::min(m_BoundingBox.m_Min.y, vertex.m_Attributes.m_Position.y);
        m_BoundingBox.m_Min.z = std::min(m_BoundingBox.m_Min.z, vertex.m_Attributes.m_Position.z);

        m_BoundingBox.m_Max.x = std::max(m_BoundingBox.m_Max.x, vertex.m_Attributes.m_Position.x);
        m_BoundingBox.m_Max.y = std::max(m_BoundingBox.m_Max.y, vertex.m_Attributes.m_Position.y);
        m_BoundingBox.m_Max.z = std::max(m_BoundingBox.m_Max.z, vertex.m_Attributes.m_Position.z);
    }
}

void Ether::Graphics::SkinnedMesh::SetPackedVertices(std::vector<VertexFormats::SkinnedVertexFormat>&& vertices)
{
    m_PackedVertices = std::move(vertices);
    m_NumVertices = m_PackedVertices.size();

    ComputeBoundingBox();
}

void Ether::Graphics::SkinnedMesh::NextFrame(const Skeleton& skeleton, const AnimationClip& animationClip)
{
    // --- CPU Skinning --- //
    // Loop animation time
    
    float animationTime = GraphicCore::GetApplicationTimeOverride();
    if (animationTime < 0)
        animationTime = Time::GetTimeSinceStartup();

    // RTCamp11 Hack (TODO)
    const float ticksPerSecond = animationClip.GetTicksPerSecond(); // TODO: Get from animation file
    const float duration = animationClip.GetTotalTicks();
    const float timeInSeconds = animationTime / 1000.0f;
    const float loopedTimeInTicks = std::fmod(timeInSeconds * ticksPerSecond, duration);
    const float loopedTimeInTicksPrev = std::fmod((timeInSeconds - Time::GetDeltaTime() / 1000.0f) * ticksPerSecond, duration);

    const SkeletonPose prevPose = skeleton.CalculatePoseFromAnimation(animationClip, loopedTimeInTicksPrev);
    const SkeletonPose pose = skeleton.CalculatePoseFromAnimation(animationClip, loopedTimeInTicks);

    std::vector<ethMatrix4x4> boneMatrices(skeleton.NumBones());
    std::vector<ethMatrix4x4> prevBoneMatrices(skeleton.NumBones());
    for (uint32_t b = 0; b < skeleton.NumBones(); ++b)
    {
        boneMatrices[b] = pose.m_GlobalInverseTransform * pose.m_GlobalBoneTransform[b] * skeleton.GetBone(b).m_InverseBindMatrix;
        prevBoneMatrices[b] = prevPose.m_GlobalInverseTransform * prevPose.m_GlobalBoneTransform[b] * skeleton.GetBone(b).m_InverseBindMatrix;
    }

    std::for_each(
        std::execution::par,
        std::begin(m_PackedVerticesOriginal),
        std::begin(m_PackedVerticesOriginal) + m_NumVertices,
        [&](const VertexFormats::SkinnedVertexFormat& src)
        {
            const uint32_t i = &src - &m_PackedVerticesOriginal[0];

            ethVector4 skinnedPos(0, 0, 0, 0);
            ethVector4 skinnedNormal(0, 0, 0, 0);
            ethVector4 prevSkinnedPos(0, 0, 0, 0);

            for (uint32_t j = 0; j < MaxBonesPerVextex; ++j)
            {
                const uint32_t boneIndex = src.m_BoneIndices[j];
                const float weight = src.m_BoneWeights[j];
                if (weight <= 0.0f || boneIndex == Graphics::InvalidBoneIndex)
                    continue;

                const ethMatrix4x4 finalBoneMatrix = boneMatrices[boneIndex];
                const ethMatrix4x4 prevBoneMatrix = prevBoneMatrices[boneIndex];

                skinnedPos += (finalBoneMatrix * ethVector4(src.m_Attributes.m_Position.x, src.m_Attributes.m_Position.y, src.m_Attributes.m_Position.z, 1.0f)) * weight;
                skinnedNormal += (finalBoneMatrix * ethVector4(src.m_Attributes.m_Normal.x, src.m_Attributes.m_Normal.y, src.m_Attributes.m_Normal.z, 0.0f)) * weight;
                prevSkinnedPos += (prevBoneMatrix * ethVector4(src.m_Attributes.m_Position.x, src.m_Attributes.m_Position.y, src.m_Attributes.m_Position.z, 1.0f)) * weight;
            }

            m_PackedVertices[i].m_Attributes.m_Position = skinnedPos.Resize<3>();
            m_PackedVertices[i].m_Attributes.m_PrevPosition = prevSkinnedPos.Resize<3>();
            m_PackedVertices[i].m_Attributes.m_Normal = skinnedNormal.Resize<3>().Normalized();
        });
}

void Ether::Graphics::SkinnedMesh::UpdateGpuResources(CommandContext& ctx)
{
    const size_t vertexBufferSize = m_NumVertices * GetVertexStride();

    m_GPUCompatiblePackedVertices.resize(m_PackedVertices.size());
    for (uint32_t i = 0; i < m_PackedVertices.size(); ++i)
    {
        m_GPUCompatiblePackedVertices[i].m_Attributes = m_PackedVertices[i].m_Attributes;
    }

    // Copy CPU-skinned data into staging/upload buffer
    void* mappedAddr;
    m_StagingVertexBufferResource->Map(&mappedAddr);
    memcpy(mappedAddr, m_GPUCompatiblePackedVertices.data(), vertexBufferSize);
    m_StagingVertexBufferResource->Unmap();

    ctx.TransitionResource(*m_VertexBufferResource, RhiResourceState::CopyDest);
    ctx.CopyBufferRegion(*m_StagingVertexBufferResource, *m_VertexBufferResource, vertexBufferSize);
    ctx.TransitionResource(*m_VertexBufferResource, RhiResourceState::Common);

    // Update BVH
    RefitAccelerationStructure(ctx);
}

void Ether::Graphics::SkinnedMesh::CreateStagingVertexBuffer()
{
    size_t bufferSize = m_NumVertices * GetVertexStride();
    RhiCommitedResourceDesc desc = {};
    desc.m_Name = "SkinnedMesh::StagingUploadBuffer";
    desc.m_HeapType = RhiHeapType::Upload;
    desc.m_State = RhiResourceState::GenericRead;
    desc.m_ResourceDesc = RhiCreateBufferResourceDesc(bufferSize);
    m_StagingVertexBufferResource = GraphicCore::GetDevice().CreateCommittedResource(desc);
}

void Ether::Graphics::SkinnedMesh::RefitAccelerationStructure(CommandContext& ctx)
{
    if (!m_AccelerationStructure)
        return;

    ctx.PushMarker("Refit BLAS");
    ctx.RefitBottomLevelAccelerationStructure(*m_AccelerationStructure);
    ctx.PopMarker();
}

