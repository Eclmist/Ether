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

#include "graphicrenderer.h"
#include "graphic/rhi/rhicommandqueue.h"

#ifdef _DEBUG
#include <dxgidebug.h>
#endif

ETH_NAMESPACE_BEGIN

GraphicRenderer::GraphicRenderer()
    : m_GraphicContext(L"GraphicRenderer::GraphicContext")
    , m_ResourceContext(m_GraphicContext, L"GraphicRenderer::ResourceContext")
{
    LogGraphicsInfo("Initializing Graphic Renderer");
    m_Scheduler.RegisterRenderPasses();
}

GraphicRenderer::~GraphicRenderer()
{
    GraphicCore::FlushGpu();
}

void GraphicRenderer::WaitForPresent()
{
    OPTICK_EVENT("Renderer - WaitForPresent");
    m_GraphicContext.GetCommandQueue()->StallForFence(GraphicCore::GetGraphicDisplay().GetCurrentBackBufferFence());
}

void GraphicRenderer::Render()
{
    OPTICK_EVENT("Renderer - Render");
    
    GraphicDisplay& gfxDisplay = GraphicCore::GetGraphicDisplay();
    m_GraphicContext.TransitionResource(gfxDisplay.GetCurrentBackBuffer(), RHIResourceState::RenderTarget);
    m_GraphicContext.SetViewport(gfxDisplay.GetViewport()); // TODO: Figure out why this must be set again in the render pass

    m_Scheduler.ScheduleRenderPasses(m_GraphicContext, m_ResourceContext);
    m_Scheduler.RenderPasses(m_GraphicContext, m_ResourceContext);
}

void GraphicRenderer::Present()
{
    OPTICK_EVENT("Renderer - Present");
    GraphicDisplay& gfxDisplay = GraphicCore::GetGraphicDisplay();

    m_GraphicContext.TransitionResource(gfxDisplay.GetCurrentBackBuffer(), RHIResourceState::Present);
    m_GraphicContext.FinalizeAndExecute();
    m_GraphicContext.Reset();
    m_ResourceContext.Reset();
    gfxDisplay.SetCurrentBackBufferFence(m_GraphicContext.GetCompletionFenceValue());
    gfxDisplay.Present();
}

void GraphicRenderer::CleanUp()
{
    OPTICK_EVENT("Renderer - CleanUp");
    m_PendingVisualNodes.clear();
}

void GraphicRenderer::DrawNode(VisualNode* node)
{
    OPTICK_EVENT("Renderer - DrawNode");
    m_PendingVisualNodes.push_back(node);
}

ETH_NAMESPACE_END

