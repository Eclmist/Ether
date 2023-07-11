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

#include "proceduralskyproducer.h"

#include "graphics/graphiccore.h"
#include "graphics/shaders/common/globalconstants.h"

DEFINE_GFX_PA(ProceduralSkyProducer)
DEFINE_GFX_RT(ProceduralSkyTexture)
DEFINE_GFX_SR(ProceduralSkyTexture)

DECLARE_GFX_CB(GlobalRingBuffer)

Ether::Graphics::ProceduralSkyProducer::ProceduralSkyProducer()
    : FullScreenProducer("ProceduralSkyProducer", "proceduralsky.hlsl")
{
}

void Ether::Graphics::ProceduralSkyProducer::GetInputOutput(ScheduleContext& schedule, ResourceContext& rc)
{
    ethVector2u resolution = GraphicCore::GetGraphicConfig().GetResolution();
    schedule.NewRT(ACCESS_GFX_RT(ProceduralSkyTexture), resolution.x, resolution.y, RhiFormat::R32G32B32A32Float);
    schedule.NewSR(ACCESS_GFX_SR(ProceduralSkyTexture), resolution.x, resolution.y, RhiFormat::R32G32B32A32Float, RhiResourceDimension::Texture2D);
    schedule.Read(ACCESS_GFX_CB(GlobalRingBuffer));
}

void Ether::Graphics::ProceduralSkyProducer::RenderFrame(GraphicContext& ctx, ResourceContext& rc)
{
    FullScreenProducer::RenderFrame(ctx, rc);
    ctx.TransitionResource(*rc.GetResource(ACCESS_GFX_RT(ProceduralSkyTexture)), RhiResourceState::RenderTarget);
    ctx.SetRenderTarget(*ACCESS_GFX_RT(ProceduralSkyTexture).Get());
    ctx.DrawInstanced(3, 1);
}

void Ether::Graphics::ProceduralSkyProducer::CreateRootSignature()
{
    std::unique_ptr<RhiRootSignatureDesc> rsDesc = GraphicCore::GetDevice().CreateRootSignatureDesc(1, 0);
    rsDesc->SetAsConstantBufferView(0, 0, RhiShaderVisibility::All);     // (b0) Global Constants
    rsDesc->SetFlags(RhiRootSignatureFlag::DirectlyIndexed);
    m_RootSignature = rsDesc->Compile((GetName() + " Root Signature").c_str());
}

