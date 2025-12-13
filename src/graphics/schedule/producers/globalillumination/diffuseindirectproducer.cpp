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

#include "diffuseindirectproducer.h"

#include "graphics/graphiccore.h"
#include "graphics/shaders/common/globalconstants.h"
#include "graphics/shaders/common/irradiancefieldparams.h"

DEFINE_GFX_PA(DiffuseIndirectProducer)
DEFINE_GFX_UA_SR(DiffuseIndirectTexture)

DECLARE_GFX_SR(IrradianceFieldIrradianceAtlas)
DECLARE_GFX_SR(IrradianceFieldDepthAtlas)
DECLARE_GFX_CB(IrradianceFieldParams)
DECLARE_GFX_CB(GlobalConstants)
DECLARE_GFX_SR(GBufferTextureA)
DECLARE_GFX_SR(GBufferTextureB)
DECLARE_GFX_SR(GBufferTextureC)
DECLARE_GFX_SR(SceneDepth)

Ether::Graphics::DiffuseIndirectProducer::DiffuseIndirectProducer()
    : FullScreenComputeProducer("DiffuseIndirectProducer", "lighting\\globalillumination\\irradiancefieldgather_cs.hlsl")
{
}

void Ether::Graphics::DiffuseIndirectProducer::GetInputOutput(ScheduleContext& schedule, ResourceContext& rc)
{
    ethVector2u resolution = GraphicCore::GetGraphicConfig().GetResolution();
    schedule.NewUA(ACCESS_GFX_UA(DiffuseIndirectTexture), resolution.x, resolution.y, BackBufferHdrFormat, RhiResourceDimension::Texture2D);
    schedule.NewSR(ACCESS_GFX_SR(DiffuseIndirectTexture), resolution.x, resolution.y, BackBufferHdrFormat, RhiResourceDimension::Texture2D);

    schedule.Read(ACCESS_GFX_SR(IrradianceFieldIrradianceAtlas));
    schedule.Read(ACCESS_GFX_SR(IrradianceFieldDepthAtlas));
    schedule.Read(ACCESS_GFX_CB(IrradianceFieldParams));
    schedule.Read(ACCESS_GFX_CB(GlobalConstants));
    schedule.Read(ACCESS_GFX_SR(GBufferTextureA));
    schedule.Read(ACCESS_GFX_SR(GBufferTextureB));
    schedule.Read(ACCESS_GFX_SR(GBufferTextureC));
    schedule.Read(ACCESS_GFX_SR(SceneDepth));
}

void Ether::Graphics::DiffuseIndirectProducer::RenderFrame(GraphicContext& ctx, ResourceContext& rc)
{
    ETH_MARKER_EVENT("DiffuseIndirectProducer");

    const GraphicDisplay& gfxDisplay = GraphicCore::GetGraphicDisplay();

    FullScreenComputeProducer::RenderFrame(ctx, rc);
    ctx.Bind(ACCESS_GFX_UA(DiffuseIndirectTexture));
    ctx.Bind(ACCESS_GFX_SR(IrradianceFieldIrradianceAtlas));
    ctx.Bind(ACCESS_GFX_SR(IrradianceFieldDepthAtlas));
    ctx.Bind(ACCESS_GFX_CB(IrradianceFieldParams), AlignUp(sizeof(Shader::IrradianceFieldParams), 256) * GraphicCore::GetGraphicDisplay().GetBackBufferIndex());
    ctx.Bind(ACCESS_GFX_CB(GlobalConstants));
    ctx.Bind(ACCESS_GFX_SR(GBufferTextureA));
    ctx.Bind(ACCESS_GFX_SR(GBufferTextureB));
    ctx.Bind(ACCESS_GFX_SR(GBufferTextureC));
    ctx.Bind(ACCESS_GFX_SR(SceneDepth));
    DispatchFullscreen(ctx);
}

