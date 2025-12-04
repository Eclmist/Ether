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

#include "raytracingresourceproducer.h"

#include "graphics/graphiccore.h"
#include "graphics/shaders/common/raytracingconstants.h"
#include "graphics/resources/staticmesh.h"
#include "graphics/resources/skinnedmesh.h"

DEFINE_GFX_PA(RaytracingResourceProducer)
DEFINE_GFX_SR(RTGeometryInfo)
DEFINE_GFX_AS(RTRaytracingTlas)

DEFINE_GFX_UA(SpatialHash)
DEFINE_GFX_UA(SpatialHashAge)
DEFINE_GFX_UA(SpatialHashPayload)

Ether::Graphics::RaytracingResourceProducer::RaytracingResourceProducer()
    : GraphicProducer("RaytracingResourceProducer")
{
}

void Ether::Graphics::RaytracingResourceProducer::Initialize(ResourceContext& rc)
{
}

void Ether::Graphics::RaytracingResourceProducer::GetInputOutput(ScheduleContext& schedule, ResourceContext& rc)
{
    ethVector2u resolution = GraphicCore::GetGraphicConfig().GetResolution();
    uint32_t numRTVisuals = GraphicCore::GetGraphicRenderer().GetThreadedRenderData().m_RaytracingVisuals.size();

    schedule.NewSR(ACCESS_GFX_SR(RTGeometryInfo), sizeof(Shader::GeometryInfo) * numRTVisuals, 0, RhiFormat::Unknown, RhiResourceDimension::StructuredBuffer, sizeof(Shader::GeometryInfo));
    schedule.NewAS(ACCESS_GFX_AS(RTRaytracingTlas), GraphicCore::GetGraphicRenderer().GetThreadedRenderData().m_RaytracingVisuals);

    /* Spatial Hashing Prototype */
    const uint32_t numHashEntries = std::clamp(GraphicCore::GetGraphicConfig().m_SpatialHashSize, 1 << 10, 1 << 18);
    schedule.NewUA(ACCESS_GFX_UA(SpatialHash), sizeof(uint32_t) * numHashEntries, 0, RhiFormat::Unknown, RhiResourceDimension::StructuredBuffer, sizeof(uint32_t));
    schedule.NewUA(ACCESS_GFX_UA(SpatialHashAge), sizeof(uint32_t) * numHashEntries, 0, RhiFormat::Unknown, RhiResourceDimension::StructuredBuffer, sizeof(uint32_t));
    schedule.NewUA(ACCESS_GFX_UA(SpatialHashPayload), sizeof(Shader::SpatialHashPayload) * numHashEntries, 0, RhiFormat::Unknown, RhiResourceDimension::StructuredBuffer, sizeof(Shader::SpatialHashPayload));
}

void Ether::Graphics::RaytracingResourceProducer::RenderFrame(GraphicContext& ctx, ResourceContext& rc)
{    
    ETH_MARKER_EVENT("RaytracingResourceProducer");

    const std::vector<Visual>& raytracedVisuals = GraphicCore::GetGraphicRenderer().GetThreadedRenderData().m_RaytracingVisuals;

    auto alloc = GetFrameAllocator().Allocate({ sizeof(Shader::GeometryInfo) * raytracedVisuals.size(), 256 });
    Shader::GeometryInfo* geometryInfos = (Shader::GeometryInfo*)alloc->GetCpuHandle();
    for (uint32_t i = 0; i < raytracedVisuals.size(); ++i)
    {
        geometryInfos[i].m_VBDescriptorIndex = raytracedVisuals[i].m_Mesh->GetVertexBufferSrvIndex();
        geometryInfos[i].m_IBDescriptorIndex = raytracedVisuals[i].m_Mesh->GetIndexBufferSrvIndex();
        geometryInfos[i].m_MaterialIndex = raytracedVisuals[i].m_Material->GetTransientMaterialIdx();
    }

    ctx.PushMarker("Upload Raytracing Geometry Info");
    ctx.CopyBufferRegion(dynamic_cast<UploadBufferAllocation&>(*alloc).GetResource(), *rc.GetResource(ACCESS_GFX_SR(RTGeometryInfo)), sizeof(Shader::GeometryInfo) * raytracedVisuals.size(), 0, 0);
    ctx.PopMarker();
}


bool Ether::Graphics::RaytracingResourceProducer::IsEnabled()
{
    if (!GraphicCore::GetGraphicConfig().m_IsRaytracingEnabled)
        return false;

    return true;
}

