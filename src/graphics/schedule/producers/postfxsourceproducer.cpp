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

#include "postfxsourceproducer.h"

#include "graphics/graphiccore.h"
#include "graphics/shaders/common/globalconstants.h"

DEFINE_GFX_PA(PostFxSourceProducer)
DEFINE_GFX_SR(PostFxSourceTexture)
DEFINE_GFX_UA(PostFxSourceTexture)

DECLARE_GFX_SR(SceneColor)

Ether::Graphics::PostFxSourceProducer::PostFxSourceProducer()
    : PostProcessProducer("PostFxSourceProducer", "postprocess\\postprocess_cs.hlsl")
{
}

void Ether::Graphics::PostFxSourceProducer::GetInputOutput(ScheduleContext& schedule, ResourceContext& rc)
{
    ethVector2u resolution = GraphicCore::GetGraphicConfig().GetResolution();
    schedule.NewSR(ACCESS_GFX_SR(PostFxSourceTexture), resolution.x, resolution.y, BackBufferHdrFormat, RhiResourceDimension::Texture2D);
    schedule.NewUA(ACCESS_GFX_UA(PostFxSourceTexture), resolution.x, resolution.y, BackBufferHdrFormat, RhiResourceDimension::Texture2D);
    schedule.Read(ACCESS_GFX_SR(SceneColor));
}

void Ether::Graphics::PostFxSourceProducer::RenderFrame(GraphicContext& ctx, ResourceContext& rc)
{
    ETH_MARKER_EVENT("PostFxSourceProducer");

    PostProcessProducer::RenderFrame(ctx, rc);
    ctx.Bind(ACCESS_GFX_SR(SceneColor));
    ctx.Bind(ACCESS_GFX_UA(PostFxSourceTexture));
    DispatchFullscreen(ctx);
}

