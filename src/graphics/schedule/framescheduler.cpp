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

#include "graphics/schedule/gbufferproducer.h"
#include "graphics/schedule/globalconstantsproducer.h"

Ether::Graphics::GBufferProducer* g_GBufferProducer;
Ether::Graphics::GlobalConstantsProducer* g_GlobalConstantsProducer;

// TEMP ============================================
#include "graphics/schedule/TempRaytracingFrameDump.h"
Ether::Graphics::TempRaytracingFrameDump* g_TempRaytracingPass;
// =================================================

Ether::Graphics::FrameScheduler::FrameScheduler()
{
    // This should be where render passes should be scheduled.

    // For now, just setup the temp frame dump here
    g_GBufferProducer = new GBufferProducer();
    g_GlobalConstantsProducer = new GlobalConstantsProducer();
    g_TempRaytracingPass = new TempRaytracingFrameDump();

    // Also for now, add imgui here
    m_ImguiWrapper = RhiImguiWrapper::InitForPlatform();
}

Ether::Graphics::FrameScheduler::~FrameScheduler()
{
    delete g_GBufferProducer;
    delete g_GlobalConstantsProducer;
    delete g_TempRaytracingPass;
}

void Ether::Graphics::FrameScheduler::PrecompilePipelineStates()
{
    ETH_MARKER_EVENT("Frame Scheduler - Precompile pipeline states");

    // Gather all PSOs declared in render passes
    // Compile what needs compiling (which should be everything)
    // Put it into resource context (unordered_map cache)

    g_TempRaytracingPass->Initialize(m_ResourceContext);
    g_GBufferProducer->Initialize(m_ResourceContext);
    g_GlobalConstantsProducer->Initialize(m_ResourceContext);
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

    g_TempRaytracingPass->FrameSetup(m_ResourceContext);
    g_GBufferProducer->FrameSetup(m_ResourceContext);
    g_GlobalConstantsProducer->FrameSetup(m_ResourceContext);
}

void Ether::Graphics::FrameScheduler::RenderSingleThreaded(GraphicContext& context)
{
    ETH_MARKER_EVENT("Frame Scheduler - Render Single Threaded");

    GraphicDisplay& gfxDisplay = GraphicCore::GetGraphicDisplay();
    // Call some function to build command lists in all the passes
    // Submit command lists in the correct execution order

    context.Reset();

    g_GlobalConstantsProducer->Reset();
    g_GlobalConstantsProducer->Render(context, m_ResourceContext);

    g_GBufferProducer->Reset();
    g_GBufferProducer->Render(context, m_ResourceContext);

    // For now, just render the frame dump
    if (GraphicCore::GetGraphicConfig().m_RaytracingDebugMode)
    {
        g_TempRaytracingPass->Reset();
        g_TempRaytracingPass->Render(context, m_ResourceContext);
    }

    context.FinalizeAndExecute();

    m_ImguiWrapper->Render();

    // The following can be moved into its own render pass
    context.Reset();
    context.TransitionResource(gfxDisplay.GetBackBuffer(), RhiResourceState::Present);
    context.FinalizeAndExecute();
}

void Ether::Graphics::FrameScheduler::RenderMultiThreaded(GraphicContext& context)
{
}
