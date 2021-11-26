/*
    This file is part of Ether, an open-source DirectX 12 renderer.

    Copyright (c) 2020-2021 Samuel Van Allen - All rights reserved.

    Ether is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "clearframebufferpass.h"

ETH_NAMESPACE_BEGIN

ClearFrameBufferPass::ClearFrameBufferPass()
    : RenderPass("Clear Frame Buffer Pass")
{
}

void ClearFrameBufferPass::RegisterInputOutput()
{

}

void ClearFrameBufferPass::Render(GraphicContext& context)
{
    OPTICK_EVENT("ClearFrameBufferPass - Render");

    EngineConfig& config = EngineCore::GetEngineConfig();
    GraphicDisplay& gfxDisplay = GraphicCore::GetGraphicDisplay();
    context.ClearColor(*gfxDisplay.GetCurrentBackBuffer(), config.m_ClearColor);
    context.ClearDepth(*gfxDisplay.GetDepthBuffer(), 1.0f);
    context.FinalizeAndExecute();
    context.Reset();
}

ETH_NAMESPACE_END

