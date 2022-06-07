/*
    This file is part of Ether, an open-source DirectX 12 renderer.

    Copyright (c) 2020-2022 Samuel Huang - All rights reserved.

    Ether is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "depthstencilproducer.h"

ETH_NAMESPACE_BEGIN

DEFINE_GFX_PASS(DepthStencilProducer);

DEFINE_GFX_RESOURCE(DepthStencilTexture);
DEFINE_GFX_DSV(DepthStencilTexture);

DepthStencilProducer::DepthStencilProducer()
    : RenderPass("Depth/Stencil Producer")
{
}

void DepthStencilProducer::Initialize()
{
}

void DepthStencilProducer::RegisterInputOutput(GraphicContext& context, ResourceContext& rc)
{
    RhiViewportDesc vp = context.GetViewport();
    rc.CreateDepthStencilResource(vp.m_Width, vp.m_Height, RhiFormat::D32Float, GFX_RESOURCE(DepthStencilTexture));
    rc.CreateDepthStencilView(GFX_RESOURCE(DepthStencilTexture), GFX_DSV(DepthStencilTexture));
}

void DepthStencilProducer::Render(GraphicContext& context, ResourceContext& rc)
{
    context.ClearDepthStencil(GFX_DSV(DepthStencilTexture), 0.0f, 0.0f);
    context.FinalizeAndExecute();
    context.Reset();
}

ETH_NAMESPACE_END

