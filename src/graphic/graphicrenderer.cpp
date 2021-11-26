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

#include "graphicrenderer.h"

#ifdef _DEBUG
#include <dxgidebug.h>
#endif

ETH_NAMESPACE_BEGIN

GraphicRenderer::GraphicRenderer()
{
    LogGraphicsInfo("Initializing Graphic Renderer");
}

GraphicRenderer::~GraphicRenderer()
{
    GraphicCore::FlushGpu();
}

void GraphicRenderer::WaitForPresent()
{
    OPTICK_EVENT("Renderer - WaitForPresent");
    m_Context.GetCommandQueue().StallForFence(GraphicCore::GetGraphicDisplay().GetCurrentBackBufferFence());
}

void GraphicRenderer::Render()
{
    OPTICK_EVENT("Renderer - Render");
    GraphicDisplay& gfxDisplay = GraphicCore::GetGraphicDisplay();
    m_Context.TransitionResource(*gfxDisplay.GetCurrentBackBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET);

    GraphicCore::GetGraphicScheduler().RenderPasses(m_Context);
}

void GraphicRenderer::Present()
{
    OPTICK_EVENT("Renderer - Present");
    GraphicDisplay& gfxDisplay = GraphicCore::GetGraphicDisplay();

    m_Context.TransitionResource(*gfxDisplay.GetCurrentBackBuffer(), D3D12_RESOURCE_STATE_PRESENT);
    m_Context.FinalizeAndExecute();
    m_Context.Reset();
    gfxDisplay.SetCurrentBackBufferFence(m_Context.GetCompletionFenceValue());
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

