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

#include "clearframebufferpass.h"

ETH_NAMESPACE_BEGIN

DEFINE_GFX_PASS(ClearFrameBufferPass);
DECLARE_GFX_DSV(GBufferDepthTexture);

ClearFrameBufferPass::ClearFrameBufferPass()
    : RenderPass("Clear Frame Buffer Pass")
{
}

void ClearFrameBufferPass::RegisterInputOutput(GraphicContext& context, ResourceContext& rc)
{

}

void ClearFrameBufferPass::Render(GraphicContext& context, ResourceContext& rc)
{
    OPTICK_EVENT("Clear Frame Buffer Pass - Render");

    EngineConfig& config = EngineCore::GetEngineConfig();
    GraphicDisplay& gfxDisplay = GraphicCore::GetGraphicDisplay();
    context.ClearColor(gfxDisplay.GetCurrentBackBufferRTV(), config.m_ClearColor);
    context.ClearDepthStencil(GFX_DSV(GBufferDepthTexture), 1.0f, 0.0f);
    context.FinalizeAndExecute();
    context.Reset();
}

ETH_NAMESPACE_END

