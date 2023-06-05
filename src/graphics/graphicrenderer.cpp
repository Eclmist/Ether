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

#include "graphics/graphiccore.h"
#include "graphics/graphicrenderer.h"
#include "graphics/resources/material.h"
#include "graphics/rhi/rhishader.h"

Ether::Graphics::GraphicRenderer::GraphicRenderer()
    : m_Context("GraphicRenderer - Main Graphics Context")
    , m_FrameNumber(0)
{
    LogGraphicsInfo("Initializing Graphic Renderer");
    m_Scheduler.PrecompilePipelineStates();
}

void Ether::Graphics::GraphicRenderer::WaitForPresent()
{
    ETH_MARKER_EVENT("Renderer - Wait for Present");
    GraphicDisplay& gfxDisplay = GraphicCore::GetGraphicDisplay();
    m_Context.GetCommandQueue().StallForFence(gfxDisplay.GetBackBufferFence());

    m_FrameNumber++;
}

void Ether::Graphics::GraphicRenderer::Render()
{
    ETH_MARKER_EVENT("Renderer - Render");
    m_Scheduler.BuildSchedule();
    m_Scheduler.RenderSingleThreaded(m_Context);
}

void Ether::Graphics::GraphicRenderer::Present()
{
    ETH_MARKER_EVENT("Renderer - Present");
    GraphicDisplay& gfxDisplay = GraphicCore::GetGraphicDisplay();
    gfxDisplay.SetCurrentBackBufferFence(m_Context.GetCommandQueue().GetFinalFenceValue());
    gfxDisplay.Present();
}
