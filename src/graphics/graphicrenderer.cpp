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

#include "graphics/graphiccore.h"
#include "graphics/graphicrenderer.h"
#include "graphics/resources/material.h"
#include "graphics/rhi/rhiresource.h"
#include "graphics/rhi/rhishader.h"

Ether::Graphics::GraphicRenderer::GraphicRenderer()
    : m_FrameNumber(0)
    , m_ExportRequested(false)
{
    LogGraphicsInfo("Initializing Graphic Renderer");
    m_Scheduler.PrecompilePipelineStates();
}

Ether::Graphics::RhiResource* Ether::Graphics::GraphicRenderer::GetFrameResource(const RhiResourceView* resourceView) const
{
    return m_Scheduler.m_ResourceContext.GetResource(resourceView);
}

Ether::Graphics::RenderData& Ether::Graphics::GraphicRenderer::GetThreadedRenderData()
{
    if (GraphicCore::GetRenderThread().IsGraphicsThread())
    {
        return m_RenderData[(m_FrameNumber + 1) % 2];
    }
    else
    {
        return m_RenderData[m_FrameNumber % 2];
    }
}

Ether::Graphics::RenderData& Ether::Graphics::GraphicRenderer::GetRenderData(uint32_t index)
{
    return m_RenderData[index];
}
 
void Ether::Graphics::GraphicRenderer::EnqueueImGuiCommand(std::function<void()> cmd)
{
    m_Scheduler.m_ImguiWrapper->EnqueueExternalCommand(cmd);
}

ImGuiContext* Ether::Graphics::GraphicRenderer::GetImGuiContext()
{
    return m_Scheduler.m_ImguiWrapper->GetImGuiContext();
}

void Ether::Graphics::GraphicRenderer::ClearAllRenderData()
{
    GraphicCore::GetBindlessDescriptorManager().Reset();
    m_RenderData[0] = RenderData();
    m_RenderData[1] = RenderData();
    m_Scheduler.m_ResourceContext.Reset();
}

void Ether::Graphics::GraphicRenderer::WaitForPresent()
{
    ETH_MARKER_EVENT("Renderer - Waiting for GPU (Present)");
    GraphicDisplay& gfxDisplay = GraphicCore::GetGraphicDisplay();
    GraphicCore::GetCommandManager().GetGraphicQueue().StallForFence(gfxDisplay.GetBackBufferFence());
}

void Ether::Graphics::GraphicRenderer::Render()
{
    ETH_MARKER_EVENT("GraphicRenderer - Render");
    GraphicContext gfxContext("GraphicRenderer - Single Threaded Render Context");
    gfxContext.Reset();
    m_Scheduler.BuildSchedule(gfxContext);
    m_Scheduler.RenderSingleThreaded(gfxContext);
}

void Ether::Graphics::GraphicRenderer::Present()
{
    ETH_MARKER_EVENT("Renderer - Present");
    GraphicDisplay& gfxDisplay = GraphicCore::GetGraphicDisplay();
    gfxDisplay.SetCurrentBackBufferFence(GraphicCore::GetCommandManager().GetGraphicQueue().GetFinalFenceValue());
    gfxDisplay.Present();
}

void Ether::Graphics::GraphicRenderer::Cleanup()
{
    AssertGraphics(
        !GraphicCore::GetRenderThread().IsGraphicsThreadEnabled() || 
        !GraphicCore::GetRenderThread().IsGraphicsThread(), 
        "Render data should only be cleared by the game thread");

    GetThreadedRenderData().m_Visuals.clear();
    GetThreadedRenderData().m_VisualBatches.clear();
    GetThreadedRenderData().m_RaytracingVisuals.clear();
}

