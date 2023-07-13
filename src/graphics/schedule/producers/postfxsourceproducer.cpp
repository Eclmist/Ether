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

#include "postfxsourceproducer.h"

#include "graphics/graphiccore.h"
#include "graphics/shaders/common/globalconstants.h"

DEFINE_GFX_PA(PostFxSourceProducer)
DEFINE_GFX_SR(PostFxSourceTexture)
DEFINE_GFX_UA(PostFxSourceTexture)

DECLARE_GFX_SR(LightingCompositeTexture)

Ether::Graphics::PostFxSourceProducer::PostFxSourceProducer()
    : PostProcessProducer("PostFxSourceProducer", "postprocess\\postprocess.hlsl")
{
}

void Ether::Graphics::PostFxSourceProducer::GetInputOutput(ScheduleContext& schedule, ResourceContext& rc)
{
    ethVector2u resolution = GraphicCore::GetGraphicConfig().GetResolution();
    schedule.NewSR(ACCESS_GFX_SR(PostFxSourceTexture), resolution.x, resolution.y, RhiFormat::R32G32B32A32Float, RhiResourceDimension::Texture2D);
    schedule.NewUA(ACCESS_GFX_UA(PostFxSourceTexture), resolution.x, resolution.y, RhiFormat::R32G32B32A32Float, RhiResourceDimension::Texture2D);
    schedule.Read(ACCESS_GFX_SR(LightingCompositeTexture));
}

void Ether::Graphics::PostFxSourceProducer::RenderFrame(GraphicContext& ctx, ResourceContext& rc)
{
    PostProcessProducer::RenderFrame(ctx, rc);

    ctx.TransitionResource(*rc.GetResource(ACCESS_GFX_SR(LightingCompositeTexture)), RhiResourceState::Common);
    ctx.TransitionResource(*rc.GetResource(ACCESS_GFX_UA(PostFxSourceTexture)), RhiResourceState::UnorderedAccess);
    ctx.SetComputeRootDescriptorTable(1, ACCESS_GFX_SR(LightingCompositeTexture)->GetGpuAddress());
    ctx.SetComputeRootDescriptorTable(2, ACCESS_GFX_UA(PostFxSourceTexture)->GetGpuAddress());
    DispatchFullscreen(ctx);
}

void Ether::Graphics::PostFxSourceProducer::CreateRootSignature()
{
    std::unique_ptr<RhiRootSignatureDesc> rsDesc = GraphicCore::GetDevice().CreateRootSignatureDesc(3, 0);
    rsDesc->SetAsConstantBufferView(0, 0, RhiShaderVisibility::All);     // (b0) Global Constants
    rsDesc->SetAsDescriptorTable(1, 1, RhiShaderVisibility::All);
    rsDesc->SetDescriptorTableRange(1, RhiDescriptorType::Srv, 1, 0, 0); // (t0) SourceTexture
    rsDesc->SetAsDescriptorTable(2, 1, RhiShaderVisibility::All);
    rsDesc->SetDescriptorTableRange(2, RhiDescriptorType::Uav, 1, 0, 0); // (t0) DestinationTexture
    rsDesc->SetFlags(RhiRootSignatureFlag::DirectlyIndexed);
    m_RootSignature = rsDesc->Compile((GetName() + " Root Signature").c_str());
}

