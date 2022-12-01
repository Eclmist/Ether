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

#include "graphics/core.h"
#include "graphics/graphicrenderer.h"
#include "graphics/resources/material.h"
#include "graphics/rhi/rhishader.h"

#include "graphics/rendergraph/tempframedump.h"

Ether::Graphics::TempFrameDump g_TempFrameDump;

Ether::Graphics::GraphicRenderer::GraphicRenderer()
    : m_Context("GraphicRenderer::GraphicContext")
{
    LogGraphicsInfo("Initializing Graphic Renderer");
}

void Ether::Graphics::GraphicRenderer::WaitForPresent()
{
    ETH_MARKER_EVENT("Renderer - WaitForPresent");
    GraphicDisplay& gfxDisplay = Core::GetGraphicDisplay();
    m_Context.GetCommandQueue().StallForFence(gfxDisplay.GetBackBufferFence());
}

void Ether::Graphics::GraphicRenderer::Render()
{
    ETH_MARKER_EVENT("Renderer - Render");

    if (Core::GetFrameNumber() == 0)
        g_TempFrameDump.Setup();

    g_TempFrameDump.Render(m_Context);

    Core::GetImguiWrapper().Render();
}

void Ether::Graphics::GraphicRenderer::Present()
{
    ETH_MARKER_EVENT("Renderer - Present");
    GraphicDisplay& gfxDisplay = Core::GetGraphicDisplay();

    m_Context.TransitionResource(gfxDisplay.GetBackBuffer(), RhiResourceState::Present);
    m_Context.FinalizeAndExecute();
    m_Context.Reset();

    gfxDisplay.SetCurrentBackBufferFence(m_Context.GetCommandQueue().GetFinalFenceValue());
    gfxDisplay.Present();

    g_TempFrameDump.Reset();
}

