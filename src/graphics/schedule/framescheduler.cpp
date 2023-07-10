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
#include "graphics/schedule/framecompositeproducer.h"

DECLARE_GFX_PA(GlobalConstantsProducer)
DECLARE_GFX_PA(GBufferProducer)
DECLARE_GFX_PA(FrameCompositeProducer)

// TEMP ============================================
#include "graphics/schedule/TempRaytracingFrameDump.h"
DECLARE_GFX_PA(TempRaytracingFrameDump)
// =================================================

Ether::Graphics::FrameScheduler::FrameScheduler()
{
    // This should be where render passes should be scheduled.
    ACCESS_GFX_PA(GlobalConstantsProducer).Set(new GlobalConstantsProducer());
    ACCESS_GFX_PA(GBufferProducer).Set(new GBufferProducer());
    ACCESS_GFX_PA(TempRaytracingFrameDump).Set(new TempRaytracingFrameDump());
    ACCESS_GFX_PA(FrameCompositeProducer).Set(new FrameCompositeProducer());

    GraphicCore::GetRenderGraphManager().Register(ACCESS_GFX_PA(GlobalConstantsProducer));
    GraphicCore::GetRenderGraphManager().Register(ACCESS_GFX_PA(GBufferProducer));
    GraphicCore::GetRenderGraphManager().Register(ACCESS_GFX_PA(TempRaytracingFrameDump));
    GraphicCore::GetRenderGraphManager().Register(ACCESS_GFX_PA(FrameCompositeProducer));

    // Also for now, add imgui here
    m_ImguiWrapper = RhiImguiWrapper::InitForPlatform();
}

Ether::Graphics::FrameScheduler::~FrameScheduler()
{
}

void Ether::Graphics::FrameScheduler::PrecompilePipelineStates()
{
    ETH_MARKER_EVENT("Frame Scheduler - Precompile pipeline states");

    // Gather all PSOs declared in render passes
    // Compile what needs compiling (which should be everything)
    // Put it into resource context (unordered_map cache)

    ACCESS_GFX_PA(GlobalConstantsProducer).Get()->Initialize(m_ResourceContext);
    ACCESS_GFX_PA(GBufferProducer).Get()->Initialize(m_ResourceContext);
    ACCESS_GFX_PA(TempRaytracingFrameDump).Get()->Initialize(m_ResourceContext);
    ACCESS_GFX_PA(FrameCompositeProducer).Get()->Initialize(m_ResourceContext);
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

    ACCESS_GFX_PA(GlobalConstantsProducer).Get()->PrepareFrame(m_ResourceContext);
    ACCESS_GFX_PA(GBufferProducer).Get()->PrepareFrame(m_ResourceContext);
    ACCESS_GFX_PA(TempRaytracingFrameDump).Get()->PrepareFrame(m_ResourceContext);
    ACCESS_GFX_PA(FrameCompositeProducer).Get()->PrepareFrame(m_ResourceContext);
}

void Ether::Graphics::FrameScheduler::RenderSingleThreaded(GraphicContext& context)
{
    ETH_MARKER_EVENT("Frame Scheduler - Render Single Threaded");

    GraphicDisplay& gfxDisplay = GraphicCore::GetGraphicDisplay();
    // Call some function to build command lists in all the passes
    // Submit command lists in the correct execution order

    context.Reset();

    ACCESS_GFX_PA(GlobalConstantsProducer).Get()->Reset();
    ACCESS_GFX_PA(GBufferProducer).Get()->Reset();
    ACCESS_GFX_PA(TempRaytracingFrameDump).Get()->Reset();
    ACCESS_GFX_PA(FrameCompositeProducer).Get()->Reset();

    ACCESS_GFX_PA(GlobalConstantsProducer).Get()->RenderFrame(context, m_ResourceContext);
    ACCESS_GFX_PA(GBufferProducer).Get()->RenderFrame(context, m_ResourceContext);

    // For now, just render the frame dump
    if (GraphicCore::GetGraphicConfig().m_IsRaytracingEnabled)
    {
        ACCESS_GFX_PA(TempRaytracingFrameDump).Get()->RenderFrame(context, m_ResourceContext);
        ACCESS_GFX_PA(FrameCompositeProducer).Get()->RenderFrame(context, m_ResourceContext);
    }

    context.FinalizeAndExecute();

    if (GraphicCore::GetGraphicConfig().IsDebugGuiEnabled())
        m_ImguiWrapper->Render();

    // The following can be moved into its own render pass
    context.Reset();
    context.TransitionResource(gfxDisplay.GetBackBuffer(), RhiResourceState::Present);
    context.FinalizeAndExecute();
}

void Ether::Graphics::FrameScheduler::RenderMultiThreaded(GraphicContext& context)
{
}
