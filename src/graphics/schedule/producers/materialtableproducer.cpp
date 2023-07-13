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

#include "materialtableproducer.h"

#include "graphics/graphiccore.h"
#include "graphics/shaders/common/material.h"

DEFINE_GFX_PA(MaterialTableProducer)
DEFINE_GFX_SR(MaterialTable)

Ether::Graphics::MaterialTableProducer::MaterialTableProducer()
    : GraphicProducer("MaterialTableProducer")
{
}

void Ether::Graphics::MaterialTableProducer::Initialize(ResourceContext& rc)
{
}

void Ether::Graphics::MaterialTableProducer::GetInputOutput(ScheduleContext& schedule, ResourceContext& rc)
{
    uint32_t numMaterials = GraphicCore::GetGraphicRenderer().GetRenderData().m_VisualBatches.size();
    schedule.NewSR(ACCESS_GFX_SR(MaterialTable), sizeof(Shader::Material) * numMaterials, 0, RhiFormat::Unknown, RhiResourceDimension::StructuredBuffer, sizeof(Shader::Material));
}

void Ether::Graphics::MaterialTableProducer::RenderFrame(GraphicContext& ctx, ResourceContext& rc)
{
    const RenderData& renderData = GraphicCore::GetGraphicRenderer().GetRenderData();
    uint32_t numMaterials = renderData.m_VisualBatches.size();

    auto alloc = GetFrameAllocator().Allocate({ sizeof(Shader::Material) * numMaterials, 256 });
    Shader::Material* materials = (Shader::Material*)alloc->GetCpuHandle();
    for (uint32_t i = 0; i < numMaterials; ++i)
    {
        Material* currMat = renderData.m_VisualBatches[i].m_Material;
        materials[i].m_BaseColor = currMat->GetBaseColor();
        materials[i].m_SpecularColor = currMat->GetSpecularColor();
        materials[i].m_AlbedoTextureIndex = GraphicCore::GetBindlessDescriptorManager().GetDescriptorIndex(currMat->GetAlbedoTextureID());
        materials[i].m_NormalTextureIndex = GraphicCore::GetBindlessDescriptorManager().GetDescriptorIndex(currMat->GetNormalTextureID());
        materials[i].m_RoughnessTextureIndex = GraphicCore::GetBindlessDescriptorManager().GetDescriptorIndex(currMat->GetRoughnessTextureID());
        materials[i].m_MetalnessTextureIndex = GraphicCore::GetBindlessDescriptorManager().GetDescriptorIndex(currMat->GetMetalnessTextureID());
    }

    ctx.CopyBufferRegion(
        dynamic_cast<UploadBufferAllocation&>(*alloc).GetResource(),
        *rc.GetResource(ACCESS_GFX_SR(MaterialTable)),
        sizeof(Shader::Material) * numMaterials,
        0,
        0);
}

