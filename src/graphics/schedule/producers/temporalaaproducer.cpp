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

#include "temporalaaproducer.h"

#include "graphics/graphiccore.h"
#include "graphics/shaders/common/globalconstants.h"

DEFINE_GFX_PA(TemporalAAProducer)
DECLARE_GFX_UA(PostFxSourceTexture)
DECLARE_GFX_SR(GBufferTexture0)

Ether::Graphics::TemporalAAProducer::TemporalAAProducer()
    : PostProcessProducer("TemporalAAProducer", "postprocess\\temporalaa.hlsl")
{
}

void Ether::Graphics::TemporalAAProducer::GetInputOutput(ScheduleContext& schedule, ResourceContext& rc)
{
    schedule.Read(ACCESS_GFX_UA(PostFxSourceTexture));
    schedule.Read(ACCESS_GFX_SR(GBufferTexture0));
}

void Ether::Graphics::TemporalAAProducer::RenderFrame(GraphicContext& ctx, ResourceContext& rc)
{
    PostProcessProducer::RenderFrame(ctx, rc);

    ctx.TransitionResource(*rc.GetResource(ACCESS_GFX_UA(PostFxSourceTexture)), RhiResourceState::UnorderedAccess);
    ctx.TransitionResource(*rc.GetResource(ACCESS_GFX_SR(GBufferTexture0)), RhiResourceState::Common);
    ctx.SetComputeRootDescriptorTable(1, ACCESS_GFX_UA(PostFxSourceTexture)->GetGpuAddress());
    ctx.SetComputeRootDescriptorTable(2, ACCESS_GFX_SR(GBufferTexture0)->GetGpuAddress());
    ethVector2u resolution = GraphicCore::GetGraphicConfig().GetResolution();
    ctx.Dispatch(std::ceil(resolution.x / 32.0), std::ceil(resolution.y / 32.0), 1);
}

void Ether::Graphics::TemporalAAProducer::CreateRootSignature()
{
    std::unique_ptr<RhiRootSignatureDesc> rsDesc = GraphicCore::GetDevice().CreateRootSignatureDesc(3, 0);
    rsDesc->SetAsConstantBufferView(0, 0, RhiShaderVisibility::All);     // (b0) Global Constants
    rsDesc->SetAsDescriptorTable(1, 1, RhiShaderVisibility::All);
    rsDesc->SetDescriptorTableRange(1, RhiDescriptorType::Uav, 1, 0, 0); // (u0) PostFxSource
    rsDesc->SetAsDescriptorTable(2, 1, RhiShaderVisibility::All);
    rsDesc->SetDescriptorTableRange(2, RhiDescriptorType::Srv, 1, 0, 0); // (t0) GBufferTexture0
    rsDesc->SetFlags(RhiRootSignatureFlag::DirectlyIndexed);
    m_RootSignature = rsDesc->Compile((GetName() + " Root Signature").c_str());
}

