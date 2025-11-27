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

#include "lightingcompositeproducer.h"

#include "graphics/graphiccore.h"
#include "graphics/shaders/common/globalconstants.h"

DEFINE_GFX_PA(LightingCompositeProducer)
DEFINE_GFX_RT(SceneColor)
DEFINE_GFX_SR(SceneColor)
DEFINE_GFX_UA(SceneColor)

DECLARE_GFX_SR(GBufferTextureA)
DECLARE_GFX_SR(GBufferTextureB)
DECLARE_GFX_SR(GBufferTextureC)
DECLARE_GFX_SR(SceneDepth)
DECLARE_GFX_SR(LightingTexture)
DECLARE_GFX_SR(ProceduralSkyTexture)
DECLARE_GFX_CB(GlobalConstants)

Ether::Graphics::LightingCompositeProducer::LightingCompositeProducer()
    : FullScreenProducer("LightingCompositeProducer", "lightingcomposite_ps.hlsl")
{
}

void Ether::Graphics::LightingCompositeProducer::GetInputOutput(ScheduleContext& schedule, ResourceContext& rc)
{
    ethVector2u resolution = GraphicCore::GetGraphicConfig().GetResolution();
    schedule.NewRT(ACCESS_GFX_RT(SceneColor), resolution.x, resolution.y, BackBufferHdrFormat);
    schedule.NewSR(ACCESS_GFX_SR(SceneColor), resolution.x, resolution.y, BackBufferHdrFormat, RhiResourceDimension::Texture2D);
    schedule.NewUA(ACCESS_GFX_UA(SceneColor), resolution.x, resolution.y, BackBufferHdrFormat, RhiResourceDimension::Texture2D);

    schedule.Read(ACCESS_GFX_SR(GBufferTextureA));
    schedule.Read(ACCESS_GFX_SR(GBufferTextureB));
    schedule.Read(ACCESS_GFX_SR(GBufferTextureC));
    schedule.Read(ACCESS_GFX_SR(SceneDepth));
    schedule.Read(ACCESS_GFX_SR(LightingTexture));
    schedule.Read(ACCESS_GFX_SR(ProceduralSkyTexture));
    schedule.Read(ACCESS_GFX_CB(GlobalConstants));
}

void Ether::Graphics::LightingCompositeProducer::RenderFrame(GraphicContext& ctx, ResourceContext& rc)
{
    ETH_MARKER_EVENT("LightingCompositeProducer");

    FullScreenProducer::RenderFrame(ctx, rc);

    ctx.TransitionResource(*rc.GetResource(ACCESS_GFX_SR(GBufferTextureA)), RhiResourceState::Common);
    ctx.TransitionResource(*rc.GetResource(ACCESS_GFX_SR(GBufferTextureB)), RhiResourceState::Common);
    ctx.TransitionResource(*rc.GetResource(ACCESS_GFX_SR(GBufferTextureC)), RhiResourceState::Common);
    ctx.TransitionResource(*rc.GetResource(ACCESS_GFX_SR(SceneDepth)), RhiResourceState::Common);
    ctx.TransitionResource(*rc.GetResource(ACCESS_GFX_SR(LightingTexture)), RhiResourceState::Common);
    ctx.TransitionResource(*rc.GetResource(ACCESS_GFX_SR(ProceduralSkyTexture)), RhiResourceState::Common);
    ctx.TransitionResource(*rc.GetResource(ACCESS_GFX_RT(SceneColor)), RhiResourceState::RenderTarget);

    ctx.SetGraphicsRootDescriptorTable(1, ACCESS_GFX_SR(GBufferTextureA)->GetGpuAddress());
    ctx.SetGraphicsRootDescriptorTable(2, ACCESS_GFX_SR(GBufferTextureB)->GetGpuAddress());
    ctx.SetGraphicsRootDescriptorTable(3, ACCESS_GFX_SR(GBufferTextureC)->GetGpuAddress());
    ctx.SetGraphicsRootDescriptorTable(4, ACCESS_GFX_SR(SceneDepth)->GetGpuAddress());
    ctx.SetGraphicsRootDescriptorTable(5, ACCESS_GFX_SR(LightingTexture)->GetGpuAddress());
    ctx.SetGraphicsRootDescriptorTable(6, ACCESS_GFX_SR(ProceduralSkyTexture)->GetGpuAddress());

    ctx.SetRenderTarget(*ACCESS_GFX_RT(SceneColor).Get());
    ctx.DrawInstanced(3, 1);
}

void Ether::Graphics::LightingCompositeProducer::CreateRootSignature()
{
    std::unique_ptr<RhiRootSignatureDesc> rsDesc = GraphicCore::GetDevice().CreateRootSignatureDesc(7, 0);
    rsDesc->SetAsConstantBufferView(0, 0, RhiShaderVisibility::All);     // (b0) Global Constants

    rsDesc->SetAsDescriptorTable(1, 1, RhiShaderVisibility::All);
    rsDesc->SetDescriptorTableRange(1, RhiDescriptorType::Srv, 1, 0, 0); // (t0) GBufferTextureA
    rsDesc->SetAsDescriptorTable(2, 1, RhiShaderVisibility::All);
    rsDesc->SetDescriptorTableRange(2, RhiDescriptorType::Srv, 1, 0, 1); // (t1) GBufferTextureB
    rsDesc->SetAsDescriptorTable(3, 1, RhiShaderVisibility::All);
    rsDesc->SetDescriptorTableRange(3, RhiDescriptorType::Srv, 1, 0, 2); // (t2) GBufferTextureC
    rsDesc->SetAsDescriptorTable(4, 1, RhiShaderVisibility::All);
    rsDesc->SetDescriptorTableRange(4, RhiDescriptorType::Srv, 1, 0, 3); // (t3) SceneDepth
    rsDesc->SetAsDescriptorTable(5, 1, RhiShaderVisibility::All);
    rsDesc->SetDescriptorTableRange(5, RhiDescriptorType::Srv, 1, 0, 4); // (t4) LightingTexture
    rsDesc->SetAsDescriptorTable(6, 1, RhiShaderVisibility::All);
    rsDesc->SetDescriptorTableRange(6, RhiDescriptorType::Srv, 1, 0, 5); // (t5) ProceduralSkyTexture

    rsDesc->SetFlags(RhiRootSignatureFlag::DirectlyIndexed);
    m_RootSignature = rsDesc->Compile((GetName() + " Root Signature").c_str());
}

