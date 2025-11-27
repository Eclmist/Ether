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

#include "depthoffieldproducer.h"
#include "graphics/graphiccore.h"
#include "graphics/shaders/common/globalconstants.h"

DEFINE_GFX_PA(DepthOfFieldProducer)

DECLARE_GFX_SR(SceneDepth)
DECLARE_GFX_UA_SR(PostFxSourceTexture)

DEFINE_GFX_UA_SR(DofIntermediateTexture1)
DEFINE_GFX_UA_SR(DofIntermediateTexture2)
DEFINE_GFX_UA_SR(DofCircleOfConfusionTexture)

Ether::Graphics::DepthOfFieldProducer::DepthOfFieldProducer()
    : PostProcessProducer("DepthOfFieldProducer", "postprocess\\depthoffield_cs.hlsl")
{
}

void Ether::Graphics::DepthOfFieldProducer::GetInputOutput(ScheduleContext& schedule, ResourceContext& rc)
{
    const ethVector2u resolution = GraphicCore::GetGraphicConfig().GetResolution();

    schedule.NewUA(ACCESS_GFX_UA(DofIntermediateTexture1), resolution.x / 2.0f, resolution.y / 2.0f, BackBufferHdrFormat, RhiResourceDimension::Texture2D);
    schedule.NewSR(ACCESS_GFX_SR(DofIntermediateTexture1), resolution.x / 2.0f, resolution.y / 2.0f, BackBufferHdrFormat, RhiResourceDimension::Texture2D);
    schedule.NewUA(ACCESS_GFX_UA(DofIntermediateTexture2), resolution.x / 2.0f, resolution.y / 2.0f, BackBufferHdrFormat, RhiResourceDimension::Texture2D);
    schedule.NewSR(ACCESS_GFX_SR(DofIntermediateTexture2), resolution.x / 2.0f, resolution.y / 2.0f, BackBufferHdrFormat, RhiResourceDimension::Texture2D);
    schedule.NewUA(ACCESS_GFX_UA(DofCircleOfConfusionTexture), resolution.x, resolution.y, RhiFormat::R16Float, RhiResourceDimension::Texture2D);
    schedule.NewSR(ACCESS_GFX_SR(DofCircleOfConfusionTexture), resolution.x, resolution.y, RhiFormat::R16Float, RhiResourceDimension::Texture2D);

    schedule.Read(ACCESS_GFX_SR(SceneDepth));
    schedule.Read(ACCESS_GFX_UA(PostFxSourceTexture));
    schedule.Read(ACCESS_GFX_SR(PostFxSourceTexture));
}

void Ether::Graphics::DepthOfFieldProducer::RenderFrame(GraphicContext& ctx, ResourceContext& rc)
{
    ETH_MARKER_EVENT("DepthOfFieldProducer");

    PostProcessProducer::RenderFrame(ctx, rc);
    const GraphicConfig& config = GraphicCore::GetGraphicConfig();
    const ethVector2u resolution = config.GetResolution();
    const ethVector2u halfResolution = resolution / 2.0f;

    auto alloc = GetFrameAllocator().Allocate({ sizeof(Shader::DepthOfFieldParams), 256 });
    Shader::DepthOfFieldParams* params = (Shader::DepthOfFieldParams*)alloc->GetCpuHandle();
    params->m_Aperture = config.m_Aperture;
    params->m_FocusDistance = config.m_FocusDistance;
    params->m_FocalLength = config.m_FocalLength;
    params->m_MaxCoC = config.m_MaxCoC;
    params->m_FocusRange = config.m_FocusRange;
    ctx.Bind("DepthOfFieldParams", ((UploadBufferAllocation&)(*alloc)).GetGpuAddress());
    ctx.Bind(ACCESS_GFX_SR(SceneDepth));
    ctx.Bind(ACCESS_GFX_SR(DofCircleOfConfusionTexture));

    // Generate circle of confusion
    {
        ctx.Bind("PassIndexCB", (uint32_t)DOF_PASSINDEX_GENERATE_COC);
        ctx.Bind("SourceTexture", ACCESS_GFX_SR(PostFxSourceTexture));
        ctx.Bind("RWDestinationTexture", ACCESS_GFX_UA(DofCircleOfConfusionTexture));
        ctx.Dispatch(std::ceil(resolution.x / float(DOF_KERNEL_GROUP_SIZE_X)), std::ceil(resolution.y / float(DOF_KERNEL_GROUP_SIZE_Y)), 1);
    }

    // Generate downsampled scene color + coc
    {
        ctx.Bind("PassIndexCB", (uint32_t)DOF_PASSINDEX_PREFILTER_PASS);
        ctx.Bind("SourceTexture", ACCESS_GFX_SR(PostFxSourceTexture));
        ctx.Bind("RWDestinationTexture", ACCESS_GFX_UA(DofIntermediateTexture1));
        ctx.Dispatch(std::ceil(halfResolution.x / float(DOF_KERNEL_GROUP_SIZE_X)), std::ceil(halfResolution.y / float(DOF_KERNEL_GROUP_SIZE_Y)), 1);
    }

    // Accumulate dof / bokeh
    {
        ctx.Bind("PassIndexCB", (uint32_t)DOF_PASSINDEX_ACCUMULATE);
        ctx.Bind("SourceTexture", ACCESS_GFX_SR(DofIntermediateTexture1));
        ctx.Bind("RWDestinationTexture", ACCESS_GFX_UA(DofIntermediateTexture2));
        ctx.Dispatch(std::ceil(halfResolution.x / float(DOF_KERNEL_GROUP_SIZE_X)), std::ceil(halfResolution.y / float(DOF_KERNEL_GROUP_SIZE_Y)), 1);
    }

    // Post filter / tent filter
    {
        ctx.Bind("PassIndexCB", (uint32_t)DOF_PASSINDEX_POSTFILTER_PASS);
        ctx.Bind("SourceTexture", ACCESS_GFX_SR(DofIntermediateTexture2));
        ctx.Bind("RWDestinationTexture", ACCESS_GFX_UA(DofIntermediateTexture1));
        ctx.Dispatch(std::ceil(halfResolution.x / float(DOF_KERNEL_GROUP_SIZE_X)), std::ceil(halfResolution.y / float(DOF_KERNEL_GROUP_SIZE_Y)), 1);
    }

    // Final Composite
    {
        ctx.Bind("PassIndexCB", (uint32_t)DOF_PASSINDEX_COMPOSITE);
        ctx.Bind("DofAccumulationTexture", ACCESS_GFX_SR(DofIntermediateTexture1));
        ctx.Bind("SourceTexture", ACCESS_GFX_SR(PostFxSourceTexture));
        ctx.Bind("RWDestinationTexture", ACCESS_GFX_UA(PostFxSourceTexture));
        ctx.Dispatch(std::ceil(resolution.x / float(DOF_KERNEL_GROUP_SIZE_X)), std::ceil(resolution.y / float(DOF_KERNEL_GROUP_SIZE_Y)), 1);
    }

}

bool Ether::Graphics::DepthOfFieldProducer::IsEnabled()
{
    if (!GraphicCore::GetGraphicConfig().m_IsDofEnabled)
        return false;

    return true;
}

