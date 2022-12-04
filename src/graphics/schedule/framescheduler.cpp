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
#include "graphics/graphicdisplay.h"
#include "graphics/schedule/framescheduler.h"

// TEMP ============================================
#include "graphics/schedule/tempframedump.h"
Ether::Graphics::TempFrameDump* g_TempFrameDump;
// =================================================

Ether::Graphics::FrameScheduler::FrameScheduler()
{
    // This should be where render passes should be scheduled.

    // For now, just setup the temp frame dump here
    g_TempFrameDump = new TempFrameDump();

    // Also for now, add imgui here
    m_ImguiWrapper = RhiImguiWrapper::InitForPlatform();
}

Ether::Graphics::FrameScheduler::~FrameScheduler()
{
    delete g_TempFrameDump;
}

void Ether::Graphics::FrameScheduler::PrecompilePipelineStates()
{
    ETH_MARKER_EVENT("Frame Scheduler - Precompile pipeline states");

    // Gather all PSOs declared in render passes
    // Compile what needs compiling (which should be everything)
    // Put it into resource context (unordered_map cache)

    g_TempFrameDump->Setup(m_ResourceContext);
}

void Ether::Graphics::FrameScheduler::BuildSchedule()
{
    ETH_MARKER_EVENT("Frame Scheduler - Build Schedule");

    // Analyze all registered render passes
    //  - Figure out in what order the passes need to execute by creating a graph of dependencies
    //  - Figure out which passes can be executed in parallel (copy pipe, async compute pipe?)
    //  - Figure out resource lifetimes, and what can be aliased in a big placed resource
    //  - Batch resource barriers

    if (GraphicCore::GetGraphicConfig().GetUseShaderDaemon())
        m_ResourceContext.RecompilePipelineStates();
}

void Ether::Graphics::FrameScheduler::RenderSingleThreaded(GraphicContext& context)
{
    ETH_MARKER_EVENT("Frame Scheduler - Render Single Threaded");

    GraphicDisplay& gfxDisplay = GraphicCore::GetGraphicDisplay();

    // Call some function to build command lists in all the passes
    // Submit command lists in the correct execution order

    // For now, just render the frame dump
    g_TempFrameDump->Reset();
    g_TempFrameDump->Render(context, m_ResourceContext);
    m_ImguiWrapper->Render();

    // The following can be moved into its own render pass
    context.TransitionResource(gfxDisplay.GetBackBuffer(), RhiResourceState::Present);
    context.FinalizeAndExecute();
    context.Reset();
}

void Ether::Graphics::FrameScheduler::RenderMultiThreaded(GraphicContext& context)
{

}

