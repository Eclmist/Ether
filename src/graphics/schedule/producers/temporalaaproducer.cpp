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

#include "temporalaaproducer.h"

#include "graphics/graphiccore.h"
#include "graphics/shaders/common/globalconstants.h"

DEFINE_GFX_PA(TemporalAAProducer)
DEFINE_GFX_UA(TaaAccumulationTexture)
DEFINE_GFX_SR(TaaAccumulationTexture)

DECLARE_GFX_UA(PostFxSourceTexture)
DECLARE_GFX_SR(GBufferTextureB) // For the velocity vectors
DECLARE_GFX_SR(SceneDepth)

Ether::Graphics::TemporalAAProducer::TemporalAAProducer()
    : PostProcessProducer("TemporalAAProducer", "postprocess\\temporalaa_cs.hlsl")
{
}

void Ether::Graphics::TemporalAAProducer::GetInputOutput(ScheduleContext& schedule, ResourceContext& rc)
{
    ethVector2u resolution = GraphicCore::GetGraphicConfig().GetResolution();
    schedule.NewUA(ACCESS_GFX_UA(TaaAccumulationTexture), resolution.x, resolution.y, BackBufferHdrFormat, RhiResourceDimension::Texture2D);
    schedule.NewSR(ACCESS_GFX_SR(TaaAccumulationTexture), resolution.x, resolution.y, BackBufferHdrFormat, RhiResourceDimension::Texture2D);

    schedule.Read(ACCESS_GFX_UA(PostFxSourceTexture));
    schedule.Read(ACCESS_GFX_SR(GBufferTextureB));
    schedule.Read(ACCESS_GFX_SR(SceneDepth));
}

void Ether::Graphics::TemporalAAProducer::RenderFrame(GraphicContext& ctx, ResourceContext& rc)
{
    ETH_MARKER_EVENT("TemporalAAProducer");

    PostProcessProducer::RenderFrame(ctx, rc);

    ctx.TransitionResource(*rc.GetResource(ACCESS_GFX_UA(TaaAccumulationTexture)), RhiResourceState::UnorderedAccess);
    ctx.TransitionResource(*rc.GetResource(ACCESS_GFX_UA(PostFxSourceTexture)), RhiResourceState::UnorderedAccess);
    ctx.TransitionResource(*rc.GetResource(ACCESS_GFX_SR(GBufferTextureB)), RhiResourceState::Common);
    ctx.TransitionResource(*rc.GetResource(ACCESS_GFX_SR(SceneDepth)), RhiResourceState::Common);

    m_BindingTable->Bind(ctx, rc, ACCESS_GFX_UA(PostFxSourceTexture));
    m_BindingTable->Bind(ctx, rc, ACCESS_GFX_SR(GBufferTextureB));
    m_BindingTable->Bind(ctx, rc, ACCESS_GFX_SR(TaaAccumulationTexture));
    m_BindingTable->Bind(ctx, rc, ACCESS_GFX_SR(SceneDepth));

    DispatchFullscreen(ctx);

    ctx.TransitionResource(*rc.GetResource(ACCESS_GFX_UA(PostFxSourceTexture)), RhiResourceState::CopySrc);
    ctx.TransitionResource(*rc.GetResource(ACCESS_GFX_UA(TaaAccumulationTexture)), RhiResourceState::CopyDest);
    ctx.CopyResource(*rc.GetResource(ACCESS_GFX_UA(PostFxSourceTexture)), *rc.GetResource(ACCESS_GFX_UA(TaaAccumulationTexture)));
}

bool Ether::Graphics::TemporalAAProducer::IsEnabled()
{
    if (!GraphicCore::GetGraphicConfig().m_IsTemporalAAEnabled)
        return false;

    return true;
}


