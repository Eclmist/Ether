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

#include "proceduralskyproducer.h"

#include "graphics/graphiccore.h"
#include "graphics/shaders/common/globalconstants.h"

DEFINE_GFX_PA(ProceduralSkyProducer)
DEFINE_GFX_RT(ProceduralSkyTexture)
DEFINE_GFX_SR(ProceduralSkyTexture)

DECLARE_GFX_CB(GlobalConstants)

Ether::Graphics::ProceduralSkyProducer::ProceduralSkyProducer()
    : FullScreenProducer("ProceduralSkyProducer", "proceduralsky_ps.hlsl")
{
}

void Ether::Graphics::ProceduralSkyProducer::GetInputOutput(ScheduleContext& schedule, ResourceContext& rc)
{
    ethVector2u resolution = GraphicCore::GetGraphicConfig().GetResolution();
    schedule.NewRT(ACCESS_GFX_RT(ProceduralSkyTexture), resolution.x, resolution.y, BackBufferHdrFormat);
    schedule.NewSR(ACCESS_GFX_SR(ProceduralSkyTexture), resolution.x, resolution.y, BackBufferHdrFormat, RhiResourceDimension::Texture2D);
    schedule.Read(ACCESS_GFX_CB(GlobalConstants));
}

void Ether::Graphics::ProceduralSkyProducer::RenderFrame(GraphicContext& ctx, ResourceContext& rc)
{
    ETH_MARKER_EVENT("ProceduralSkyProducer");

    FullScreenProducer::RenderFrame(ctx, rc);
    ctx.SetRenderTarget(*ACCESS_GFX_RT(ProceduralSkyTexture).Get());
    ctx.DrawInstanced(3, 1);
}

