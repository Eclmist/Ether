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

#include "depthoffieldproducer.h"
#include "graphics/graphiccore.h"
#include "graphics/shaders/common/globalconstants.h"

DEFINE_GFX_PA(DepthOfFieldProducer)

DECLARE_GFX_SR(SceneDepth)
DECLARE_GFX_UA_SR(PostFxSourceTexture)

DEFINE_GFX_UA(DofAccumulationTexture)
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

    schedule.NewUA(ACCESS_GFX_UA(DofAccumulationTexture), resolution.x, resolution.y, BackBufferHdrFormat, RhiResourceDimension::Texture2D);
    schedule.NewUA(ACCESS_GFX_UA(DofIntermediateTexture1), resolution.x / 2.0f, resolution.y / 2.0f, BackBufferHdrFormat, RhiResourceDimension::Texture2D);
    schedule.NewSR(ACCESS_GFX_SR(DofIntermediateTexture1), resolution.x / 2.0f, resolution.y / 2.0f, BackBufferHdrFormat, RhiResourceDimension::Texture2D);
    schedule.NewUA(ACCESS_GFX_UA(DofIntermediateTexture2), resolution.x / 2.0f, resolution.y / 2.0f, BackBufferHdrFormat, RhiResourceDimension::Texture2D);
    schedule.NewSR(ACCESS_GFX_SR(DofIntermediateTexture2), resolution.x / 2.0f, resolution.y / 2.0f, BackBufferHdrFormat, RhiResourceDimension::Texture2D);
    schedule.NewUA(ACCESS_GFX_UA(DofCircleOfConfusionTexture), resolution.x, resolution.y, RhiFormat::R16Float, RhiResourceDimension::Texture2D);
    schedule.NewSR(ACCESS_GFX_SR(DofCircleOfConfusionTexture), resolution.x, resolution.y, RhiFormat::R16Float, RhiResourceDimension::Texture2D);

    schedule.Read(ACCESS_GFX_SR(PostFxSourceTexture));
    schedule.Read(ACCESS_GFX_SR(SceneDepth));

    schedule.Read(ACCESS_GFX_UA(PostFxSourceTexture));
    schedule.Read(ACCESS_GFX_SR(PostFxSourceTexture));
}

void Ether::Graphics::DepthOfFieldProducer::RenderFrame(GraphicContext& ctx, ResourceContext& rc)
{
    PostProcessProducer::RenderFrame(ctx, rc);
    const GraphicConfig& config = GraphicCore::GetGraphicConfig();
    const ethVector2u resolution = config.GetResolution();
    const ethVector2u halfResolution = resolution / 2.0f;

    // Generate circle of confusion
    {
        auto alloc = GetFrameAllocator().Allocate({ sizeof(Shader::DepthOfFieldParams), 256 });
        Shader::DepthOfFieldParams* params = (Shader::DepthOfFieldParams*)alloc->GetCpuHandle();
        BindCommonParams(*params, DOF_PASSINDEX_GENERATE_COC);

        ctx.SetComputeRootConstantBufferView(1, ((UploadBufferAllocation&)(*alloc)).GetGpuAddress());
        //ctx.SetComputeRootDescriptorTable(2, ACCESS_GFX_SR(PostFxSourceTexture)->GetGpuAddress());
        ctx.SetComputeRootDescriptorTable(3, ACCESS_GFX_SR(SceneDepth)->GetGpuAddress());
        //ctx.SetComputeRootDescriptorTable(4, ACCESS_GFX_SR(DofCircleOfConfusionTexture)->GetGpuAddress());
        ctx.SetComputeRootDescriptorTable(7, ACCESS_GFX_UA(DofCircleOfConfusionTexture)->GetGpuAddress());
        ctx.Dispatch(std::ceil(resolution.x / float(DOF_KERNEL_GROUP_SIZE_X)), std::ceil(resolution.y / float(DOF_KERNEL_GROUP_SIZE_Y)), 1);
    }

    // Generate downsampled scene color + coc
    {
        auto alloc = GetFrameAllocator().Allocate({ sizeof(Shader::DepthOfFieldParams), 256 });
        Shader::DepthOfFieldParams* params = (Shader::DepthOfFieldParams*)alloc->GetCpuHandle();
        BindCommonParams(*params, DOF_PASSINDEX_PREFILTER_PASS);

        ctx.SetComputeRootConstantBufferView(1, ((UploadBufferAllocation&)(*alloc)).GetGpuAddress());
        ctx.SetComputeRootDescriptorTable(2, ACCESS_GFX_SR(PostFxSourceTexture)->GetGpuAddress());
        ctx.SetComputeRootDescriptorTable(3, ACCESS_GFX_SR(SceneDepth)->GetGpuAddress());
        ctx.SetComputeRootDescriptorTable(4, ACCESS_GFX_SR(DofCircleOfConfusionTexture)->GetGpuAddress());
        ctx.SetComputeRootDescriptorTable(7, ACCESS_GFX_UA(DofIntermediateTexture1)->GetGpuAddress());
        ctx.Dispatch(std::ceil(halfResolution.x / float(DOF_KERNEL_GROUP_SIZE_X)), std::ceil(halfResolution.y / float(DOF_KERNEL_GROUP_SIZE_Y)), 1);
    }

    // Accumulate dof / bokeh
    {
        auto alloc = GetFrameAllocator().Allocate({ sizeof(Shader::DepthOfFieldParams), 256 });
        Shader::DepthOfFieldParams* params = (Shader::DepthOfFieldParams*)alloc->GetCpuHandle();
        BindCommonParams(*params, DOF_PASSINDEX_ACCUMULATE);

        ctx.SetComputeRootConstantBufferView(1, ((UploadBufferAllocation&)(*alloc)).GetGpuAddress());
        ctx.SetComputeRootDescriptorTable(2, ACCESS_GFX_SR(DofIntermediateTexture1)->GetGpuAddress());
        ctx.SetComputeRootDescriptorTable(7, ACCESS_GFX_UA(DofIntermediateTexture2)->GetGpuAddress());
        ctx.Dispatch(std::ceil(halfResolution.x / float(DOF_KERNEL_GROUP_SIZE_X)), std::ceil(halfResolution.y / float(DOF_KERNEL_GROUP_SIZE_Y)), 1);
    }

    // Post filter / tent filter
    {
        auto alloc = GetFrameAllocator().Allocate({ sizeof(Shader::DepthOfFieldParams), 256 });
        Shader::DepthOfFieldParams* params = (Shader::DepthOfFieldParams*)alloc->GetCpuHandle();
        BindCommonParams(*params, DOF_PASSINDEX_POSTFILTER_PASS);

        ctx.SetComputeRootConstantBufferView(1, ((UploadBufferAllocation&)(*alloc)).GetGpuAddress());
        ctx.SetComputeRootDescriptorTable(2, ACCESS_GFX_SR(DofIntermediateTexture2)->GetGpuAddress());
        ctx.SetComputeRootDescriptorTable(7, ACCESS_GFX_UA(DofIntermediateTexture1)->GetGpuAddress());
        ctx.Dispatch(
            std::ceil(halfResolution.x / float(DOF_KERNEL_GROUP_SIZE_X)),
            std::ceil(halfResolution.y / float(DOF_KERNEL_GROUP_SIZE_Y)),
            1);
    }

    // Final Composite
    {
        auto alloc = GetFrameAllocator().Allocate({ sizeof(Shader::DepthOfFieldParams), 256 });
        Shader::DepthOfFieldParams* params = (Shader::DepthOfFieldParams*)alloc->GetCpuHandle();
        BindCommonParams(*params, DOF_PASSINDEX_COMPOSITE);

        ctx.SetComputeRootConstantBufferView(1, ((UploadBufferAllocation&)(*alloc)).GetGpuAddress());
        ctx.SetComputeRootDescriptorTable(2, ACCESS_GFX_SR(PostFxSourceTexture)->GetGpuAddress());
        ctx.SetComputeRootDescriptorTable(3, ACCESS_GFX_SR(SceneDepth)->GetGpuAddress());
        ctx.SetComputeRootDescriptorTable(4, ACCESS_GFX_SR(DofCircleOfConfusionTexture)->GetGpuAddress());
        //ctx.SetComputeRootDescriptorTable(5, ACCESS_GFX_SR(DofIntermediateTexture1)->GetGpuAddress());
        ctx.SetComputeRootDescriptorTable(6, ACCESS_GFX_SR(DofIntermediateTexture1)->GetGpuAddress());
        ctx.SetComputeRootDescriptorTable(7, ACCESS_GFX_UA(PostFxSourceTexture)->GetGpuAddress());
        ctx.SetComputeRootDescriptorTable(8, ACCESS_GFX_UA(DofAccumulationTexture)->GetGpuAddress());
        ctx.Dispatch(std::ceil(resolution.x / float(DOF_KERNEL_GROUP_SIZE_X)), std::ceil(resolution.y / float(DOF_KERNEL_GROUP_SIZE_Y)), 1);
    }

}

void Ether::Graphics::DepthOfFieldProducer::BindCommonParams(Shader::DepthOfFieldParams& params, uint32_t passIndex)
{
    const GraphicConfig& config = GraphicCore::GetGraphicConfig();
    params.m_Aperture = config.m_Aperture;
    params.m_FocusDistance = config.m_FocusDistance;
    params.m_FocalLength = config.m_FocalLength;
    params.m_MaxCoC = config.m_MaxCoC;
    params.m_FocusRange = config.m_FocusRange;
    params.m_PassIndex = passIndex;
}

bool Ether::Graphics::DepthOfFieldProducer::IsEnabled()
{
    if (!GraphicCore::GetGraphicConfig().m_IsDofEnabled)
        return false;

    return true;
}

void Ether::Graphics::DepthOfFieldProducer::CreateRootSignature()
{
    std::unique_ptr<RhiRootSignatureDesc> rsDesc = GraphicCore::GetDevice().CreateRootSignatureDesc(9, 0);
    rsDesc->SetAsConstantBufferView(0, 0, RhiShaderVisibility::All); // (b0) Global Constants
    rsDesc->SetAsConstantBufferView(1, 1, RhiShaderVisibility::All); // (b1) Dof Params
    rsDesc->SetAsDescriptorTable(2, 1, RhiShaderVisibility::All);
    rsDesc->SetDescriptorTableRange(2, RhiDescriptorType::Srv, 1, 0, 0); // (t0) Source
    rsDesc->SetAsDescriptorTable(3, 1, RhiShaderVisibility::All);
    rsDesc->SetDescriptorTableRange(3, RhiDescriptorType::Srv, 1, 0, 1); // (t1) SceneDepth
    rsDesc->SetAsDescriptorTable(4, 1, RhiShaderVisibility::All);
    rsDesc->SetDescriptorTableRange(4, RhiDescriptorType::Srv, 1, 0, 2); // (t2) CoC Texture
    rsDesc->SetAsDescriptorTable(5, 1, RhiShaderVisibility::All);
    rsDesc->SetDescriptorTableRange(5, RhiDescriptorType::Srv, 1, 0, 3); // (t2) DofIntermediateTexture1
    rsDesc->SetAsDescriptorTable(6, 1, RhiShaderVisibility::All);
    rsDesc->SetDescriptorTableRange(6, RhiDescriptorType::Srv, 1, 0, 4); // (t2) DofIntermediateTexture1

    rsDesc->SetAsDescriptorTable(7, 1, RhiShaderVisibility::All);
    rsDesc->SetDescriptorTableRange(7, RhiDescriptorType::Uav, 1, 0, 0); // (u0) Destination 
    rsDesc->SetAsDescriptorTable(8, 1, RhiShaderVisibility::All);
    rsDesc->SetDescriptorTableRange(8, RhiDescriptorType::Uav, 1, 0, 1); // (u1) TemporalAccumulation 

    rsDesc->SetFlags(RhiRootSignatureFlag::DirectlyIndexed);
    m_RootSignature = rsDesc->Compile((GetName() + " Root Signature").c_str());
}

