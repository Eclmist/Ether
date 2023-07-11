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

#include "lightingcompositeproducer.h"

#include "graphics/graphiccore.h"
#include "graphics/shaders/common/globalconstants.h"

DEFINE_GFX_PA(LightingCompositeProducer)
DEFINE_GFX_RT(LightingCompositeTexture)
DEFINE_GFX_SR(LightingCompositeTexture)

DECLARE_GFX_SR(GBufferTexture0)
DECLARE_GFX_SR(GBufferTexture1)
DECLARE_GFX_SR(GBufferTexture2)
DECLARE_GFX_SR(RTLightingTexture)
DECLARE_GFX_SR(ProceduralSkyTexture)
DECLARE_GFX_CB(GlobalRingBuffer)

Ether::Graphics::LightingCompositeProducer::LightingCompositeProducer()
    : FullScreenProducer("LightingCompositeProducer", "lighting\\lightingcomposite.hlsl")
{
}

void Ether::Graphics::LightingCompositeProducer::GetInputOutput(ScheduleContext& schedule, ResourceContext& rc)
{
    ethVector2u resolution = GraphicCore::GetGraphicConfig().GetResolution();
    schedule.NewRT(ACCESS_GFX_RT(LightingCompositeTexture), resolution.x, resolution.y, RhiFormat::R32G32B32A32Float);
    schedule.NewSR(ACCESS_GFX_SR(LightingCompositeTexture), resolution.x, resolution.y, RhiFormat::R32G32B32A32Float, RhiResourceDimension::Texture2D);

    schedule.Read(ACCESS_GFX_SR(GBufferTexture0));
    schedule.Read(ACCESS_GFX_SR(GBufferTexture1));
    schedule.Read(ACCESS_GFX_SR(GBufferTexture2));
    schedule.Read(ACCESS_GFX_SR(RTLightingTexture));
    schedule.Read(ACCESS_GFX_SR(ProceduralSkyTexture));
    schedule.Read(ACCESS_GFX_CB(GlobalRingBuffer));
}

void Ether::Graphics::LightingCompositeProducer::RenderFrame(GraphicContext& ctx, ResourceContext& rc)
{
    FullScreenProducer::RenderFrame(ctx, rc);

    ctx.TransitionResource(*rc.GetResource(ACCESS_GFX_SR(GBufferTexture0)), RhiResourceState::Common);
    ctx.TransitionResource(*rc.GetResource(ACCESS_GFX_SR(GBufferTexture1)), RhiResourceState::Common);
    ctx.TransitionResource(*rc.GetResource(ACCESS_GFX_SR(GBufferTexture2)), RhiResourceState::Common);
    ctx.TransitionResource(*rc.GetResource(ACCESS_GFX_SR(RTLightingTexture)), RhiResourceState::Common);
    ctx.TransitionResource(*rc.GetResource(ACCESS_GFX_SR(ProceduralSkyTexture)), RhiResourceState::Common);
    ctx.TransitionResource(*rc.GetResource(ACCESS_GFX_RT(LightingCompositeTexture)), RhiResourceState::RenderTarget);

    ctx.SetGraphicsRootDescriptorTable(1, ACCESS_GFX_SR(GBufferTexture0)->GetGpuAddress());
    ctx.SetGraphicsRootDescriptorTable(2, ACCESS_GFX_SR(GBufferTexture1)->GetGpuAddress());
    ctx.SetGraphicsRootDescriptorTable(3, ACCESS_GFX_SR(GBufferTexture2)->GetGpuAddress());
    ctx.SetGraphicsRootDescriptorTable(4, ACCESS_GFX_SR(RTLightingTexture)->GetGpuAddress());
    ctx.SetGraphicsRootDescriptorTable(5, ACCESS_GFX_SR(ProceduralSkyTexture)->GetGpuAddress());
    ctx.SetRenderTarget(*ACCESS_GFX_RT(LightingCompositeTexture).Get());

    ctx.DrawInstanced(3, 1);
}

void Ether::Graphics::LightingCompositeProducer::CreateRootSignature()
{
    std::unique_ptr<RhiRootSignatureDesc> rsDesc = GraphicCore::GetDevice().CreateRootSignatureDesc(6, 0);
    rsDesc->SetAsConstantBufferView(0, 0, RhiShaderVisibility::All);     // (b0) Global Constants

    rsDesc->SetAsDescriptorTable(1, 1, RhiShaderVisibility::All);
    rsDesc->SetDescriptorTableRange(1, RhiDescriptorType::Srv, 1, 0, 0); // (t0) GBufferTexture0
    rsDesc->SetAsDescriptorTable(2, 1, RhiShaderVisibility::All);
    rsDesc->SetDescriptorTableRange(2, RhiDescriptorType::Srv, 1, 0, 1); // (t1) GBufferTexture1
    rsDesc->SetAsDescriptorTable(3, 1, RhiShaderVisibility::All);
    rsDesc->SetDescriptorTableRange(3, RhiDescriptorType::Srv, 1, 0, 2); // (t2) GBufferTexture2
    rsDesc->SetAsDescriptorTable(4, 1, RhiShaderVisibility::All);
    rsDesc->SetDescriptorTableRange(4, RhiDescriptorType::Srv, 1, 0, 3); // (t3) RTLightingTexture
    rsDesc->SetAsDescriptorTable(5, 1, RhiShaderVisibility::All);
    rsDesc->SetDescriptorTableRange(5, RhiDescriptorType::Srv, 1, 0, 4); // (t4) ProceduralSkyTexture

    rsDesc->SetFlags(RhiRootSignatureFlag::DirectlyIndexed);
    m_RootSignature = rsDesc->Compile((GetName() + " Root Signature").c_str());
}

