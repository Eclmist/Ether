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
#include "graphics/shaders/common/depthoffieldparams.h"

DEFINE_GFX_PA(DepthOfFieldProducer)

DECLARE_GFX_SR(GBufferTexture3) // for linear depth stored in w
DECLARE_GFX_SR(BloomTexture0)
DECLARE_GFX_SR(BloomTexture1)
DECLARE_GFX_SR(BloomTexture2)
DECLARE_GFX_SR(BloomTexture3)
DECLARE_GFX_SR(BloomTexture4)
DECLARE_GFX_SR(BloomTexture5)
DECLARE_GFX_SR(BloomTexture6)
DECLARE_GFX_SR(BloomTexture7)
DECLARE_GFX_SR(BloomTexture8)
DECLARE_GFX_UA_SR(PostFxSourceTexture)

Ether::Graphics::DepthOfFieldProducer::DepthOfFieldProducer()
    : PostProcessProducer("DepthOfFieldProducer", "postprocess\\depthoffield.hlsl")
{
}

void Ether::Graphics::DepthOfFieldProducer::GetInputOutput(ScheduleContext& schedule, ResourceContext& rc)
{
    schedule.Read(ACCESS_GFX_SR(PostFxSourceTexture));
    schedule.Read(ACCESS_GFX_SR(GBufferTexture3));
    schedule.Read(ACCESS_GFX_SR(BloomTexture0));
    schedule.Read(ACCESS_GFX_SR(BloomTexture1));
    schedule.Read(ACCESS_GFX_SR(BloomTexture2));
    schedule.Read(ACCESS_GFX_SR(BloomTexture3));
    schedule.Read(ACCESS_GFX_SR(BloomTexture4));
    schedule.Read(ACCESS_GFX_SR(BloomTexture5));
    schedule.Read(ACCESS_GFX_SR(BloomTexture6));
    schedule.Read(ACCESS_GFX_SR(BloomTexture7));
    schedule.Read(ACCESS_GFX_SR(BloomTexture8));
    schedule.Read(ACCESS_GFX_UA(PostFxSourceTexture));
    schedule.Read(ACCESS_GFX_SR(PostFxSourceTexture));
}

void Ether::Graphics::DepthOfFieldProducer::RenderFrame(GraphicContext& ctx, ResourceContext& rc)
{
    PostProcessProducer::RenderFrame(ctx, rc);
    const GraphicConfig& config = GraphicCore::GetGraphicConfig();
    const ethVector2u resolution = config.GetResolution();

    auto alloc = GetFrameAllocator().Allocate({ sizeof(Shader::DepthOfFieldParams), 256 });
    Shader::DepthOfFieldParams* params = (Shader::DepthOfFieldParams*)alloc->GetCpuHandle();
    params->m_Aperture = config.m_Aperture;
    params->m_FocusDistance = config.m_FocusDistance;
    params->m_FocalLength = config.m_FocalLength;
    params->m_MaxCoC = config.m_MaxCoC;
    ctx.SetComputeRootConstantBufferView(1, ((UploadBufferAllocation&)(*alloc)).GetGpuAddress());
    ctx.SetComputeRootDescriptorTable(2, ACCESS_GFX_SR(PostFxSourceTexture)->GetGpuAddress());
    ctx.SetComputeRootDescriptorTable(3, ACCESS_GFX_SR(GBufferTexture3)->GetGpuAddress());
    ctx.SetComputeRootDescriptorTable(4, ACCESS_GFX_SR(BloomTexture0)->GetGpuAddress());
    ctx.SetComputeRootDescriptorTable(5, ACCESS_GFX_SR(BloomTexture1)->GetGpuAddress());
    ctx.SetComputeRootDescriptorTable(6, ACCESS_GFX_SR(BloomTexture2)->GetGpuAddress());
    ctx.SetComputeRootDescriptorTable(7, ACCESS_GFX_SR(BloomTexture3)->GetGpuAddress());
    ctx.SetComputeRootDescriptorTable(8, ACCESS_GFX_SR(BloomTexture4)->GetGpuAddress());
    ctx.SetComputeRootDescriptorTable(9, ACCESS_GFX_SR(BloomTexture5)->GetGpuAddress());
    ctx.SetComputeRootDescriptorTable(10, ACCESS_GFX_SR(BloomTexture6)->GetGpuAddress());
    ctx.SetComputeRootDescriptorTable(11, ACCESS_GFX_SR(BloomTexture7)->GetGpuAddress());
    ctx.SetComputeRootDescriptorTable(12, ACCESS_GFX_SR(BloomTexture8)->GetGpuAddress());
    ctx.SetComputeRootDescriptorTable(13, ACCESS_GFX_UA(PostFxSourceTexture)->GetGpuAddress());
    ctx.Dispatch(
        std::ceil(resolution.x / float(DOF_KERNEL_GROUP_SIZE_X)),
        std::ceil(resolution.y / float(DOF_KERNEL_GROUP_SIZE_Y)),
        1);
}

bool Ether::Graphics::DepthOfFieldProducer::IsEnabled()
{
    if (!GraphicCore::GetGraphicConfig().m_IsBloomEnabled)
        return false;

    return true;
}

void Ether::Graphics::DepthOfFieldProducer::CreateRootSignature()
{
    std::unique_ptr<RhiRootSignatureDesc> rsDesc = GraphicCore::GetDevice().CreateRootSignatureDesc(14, 0);
    rsDesc->SetAsConstantBufferView(0, 0, RhiShaderVisibility::All); // (b0) Global Constants
    rsDesc->SetAsConstantBufferView(1, 1, RhiShaderVisibility::All); // (b1) Dof Params
    rsDesc->SetAsDescriptorTable(2, 1, RhiShaderVisibility::All);
    rsDesc->SetDescriptorTableRange(2, RhiDescriptorType::Srv, 1, 0, 0); // (t0) Source
    rsDesc->SetAsDescriptorTable(3, 1, RhiShaderVisibility::All);
    rsDesc->SetDescriptorTableRange(3, RhiDescriptorType::Srv, 1, 0, 1); // (t1) GBufferTexture3
    rsDesc->SetAsDescriptorTable(4, 1, RhiShaderVisibility::All);
    rsDesc->SetDescriptorTableRange(4, RhiDescriptorType::Srv, 1, 0, 2); // (t2) Bloom0
    rsDesc->SetAsDescriptorTable(5, 1, RhiShaderVisibility::All);
    rsDesc->SetDescriptorTableRange(5, RhiDescriptorType::Srv, 1, 0, 3); // (t3) Bloom1
    rsDesc->SetAsDescriptorTable(6, 1, RhiShaderVisibility::All);
    rsDesc->SetDescriptorTableRange(6, RhiDescriptorType::Srv, 1, 0, 4); // (t4) Bloom2
    rsDesc->SetAsDescriptorTable(7, 1, RhiShaderVisibility::All);
    rsDesc->SetDescriptorTableRange(7, RhiDescriptorType::Srv, 1, 0, 5); // (t5) Bloom3
    rsDesc->SetAsDescriptorTable(8, 1, RhiShaderVisibility::All);
    rsDesc->SetDescriptorTableRange(8, RhiDescriptorType::Srv, 1, 0, 6); // (t6) Bloom4
    rsDesc->SetAsDescriptorTable(9, 1, RhiShaderVisibility::All);
    rsDesc->SetDescriptorTableRange(9, RhiDescriptorType::Srv, 1, 0, 7); // (t7) Bloom5
    rsDesc->SetAsDescriptorTable(10, 1, RhiShaderVisibility::All);
    rsDesc->SetDescriptorTableRange(10, RhiDescriptorType::Srv, 1, 0, 8); // (t8) Bloom6
    rsDesc->SetAsDescriptorTable(11, 1, RhiShaderVisibility::All);
    rsDesc->SetDescriptorTableRange(11, RhiDescriptorType::Srv, 1, 0, 9); // (t9) Bloom7
    rsDesc->SetAsDescriptorTable(12, 1, RhiShaderVisibility::All);
    rsDesc->SetDescriptorTableRange(12, RhiDescriptorType::Srv, 1, 0, 10); // (t10) Bloom8
    rsDesc->SetAsDescriptorTable(13, 1, RhiShaderVisibility::All);
    rsDesc->SetDescriptorTableRange(13, RhiDescriptorType::Uav, 1, 0, 0); // (u0) Destination 

    rsDesc->SetFlags(RhiRootSignatureFlag::DirectlyIndexed);
    m_RootSignature = rsDesc->Compile((GetName() + " Root Signature").c_str());
}

