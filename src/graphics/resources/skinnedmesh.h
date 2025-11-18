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

#include "graphics/resources/mesh.h"

#define ETH_CLASS_ID_SKINNEDMESH "Graphics::SkinnedMesh"

namespace Ether::Graphics
{
class ETH_GRAPHIC_DLL SkinnedMesh : public Mesh
{
public:
    SkinnedMesh();
    ~SkinnedMesh() override = default;

public:
    inline StringID GetSkeletonGuid() const { return m_SkeletonGuid; }
    inline StringID GetAnimationGuid() const { return m_AnimationGuid; }

public:
    void Serialize(OStream& ostream) const override;
    void Deserialize(IStream& istream) override;

    void CreateGpuResources(CommandContext& ctx) override;
    void ComputeBoundingBox() override;
    void* GetPackedVertexData() override { return m_StagingVertices.data(); }
    uint32_t GetVertexStride() override { return sizeof(VertexFormats::BaseVertexFormat); }

    // TODO: Remove after GPU skinning implementation
    std::vector<VertexFormats::SkinnedVertexFormat>& GetSkinningVertices();
    std::vector<VertexFormats::BaseVertexFormat>& GetStagingVertices();

public:
    void SetPackedVertices(std::vector<VertexFormats::SkinnedVertexFormat>&& vertices);
    void SetStagingVertices(std::vector<VertexFormats::BaseVertexFormat>&& vertices);
    void SetSkeletonGuid(StringID guid) { m_SkeletonGuid = guid; }
    void SetAnimationGuid(StringID guid) { m_AnimationGuid = guid; }

public:
    void UpdateGpuResources(CommandContext& ctx);

protected:
    void InitSkinnedVertices();
    void CreateStagingVertexBuffer();
    void RefitAccelerationStructure(CommandContext& ctx);

protected:
    std::vector<VertexFormats::SkinnedVertexFormat> m_PackedVertices;

    // GPU Skinning TODO
    std::unique_ptr<RhiResource> m_StagingVertexBufferResource;
    std::vector<VertexFormats::BaseVertexFormat> m_StagingVertices;
    std::mutex m_SkinningDataMutex;

    StringID m_SkeletonGuid;
    StringID m_AnimationGuid;
};

} // namespace Ether::Graphics
