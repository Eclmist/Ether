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
#include "graphics/schedule/framescheduler.h"
#include "graphics/schedule/schedulecontext.h"

#include "graphics/schedule/producers/globalconstantsproducer.h"
#include "graphics/schedule/producers/materialtableproducer.h"
#include "graphics/schedule/producers/proceduralskyproducer.h"
#include "graphics/schedule/producers/gbufferproducer.h"
#include "graphics/schedule/producers/raytracedlightingproducer.h"
#include "graphics/schedule/producers/lightingcompositeproducer.h"
#include "graphics/schedule/producers/postfxsourceproducer.h"
#include "graphics/schedule/producers/temporalaaproducer.h"
#include "graphics/schedule/producers/finalcompositeproducer.h"

DECLARE_GFX_PA(FinalCompositeProducer)
DECLARE_GFX_PA(GBufferProducer)
DECLARE_GFX_PA(GlobalConstantsProducer)
DECLARE_GFX_PA(LightingCompositeProducer)
DECLARE_GFX_PA(MaterialTableProducer)
DECLARE_GFX_PA(PostFxSourceProducer)
DECLARE_GFX_PA(ProceduralSkyProducer)
DECLARE_GFX_PA(RaytracedLightingProducer)
DECLARE_GFX_PA(TemporalAAProducer)

Ether::Graphics::FrameScheduler::FrameScheduler()
{
    Register(ACCESS_GFX_PA(FinalCompositeProducer), new FinalCompositeProducer());
    Register(ACCESS_GFX_PA(GBufferProducer), new GBufferProducer());
    Register(ACCESS_GFX_PA(GlobalConstantsProducer), new GlobalConstantsProducer());
    Register(ACCESS_GFX_PA(LightingCompositeProducer), new LightingCompositeProducer());
    Register(ACCESS_GFX_PA(MaterialTableProducer), new MaterialTableProducer());
    Register(ACCESS_GFX_PA(PostFxSourceProducer), new PostFxSourceProducer());
    Register(ACCESS_GFX_PA(ProceduralSkyProducer), new ProceduralSkyProducer());
    Register(ACCESS_GFX_PA(RaytracedLightingProducer), new RaytracedLightingProducer());
    Register(ACCESS_GFX_PA(TemporalAAProducer), new TemporalAAProducer());

    // Also for now, add imgui here
    m_ImguiWrapper = RhiImguiWrapper::InitForPlatform();
}

Ether::Graphics::FrameScheduler::~FrameScheduler()
{
}

void Ether::Graphics::FrameScheduler::Register(GFX_STATIC::GFX_PA_TYPE& pass, GraphicProducer* producer)
{
    AssertGraphics(m_RegisteredProducers.find(pass.GetName()) == m_RegisteredProducers.end(), "RenderPass already registered");
    AssertGraphics(producer != nullptr, "Cannot register null producer");

    pass.Create(producer);
    m_RegisteredProducers.emplace(pass.GetName(), pass.Get());
}

void Ether::Graphics::FrameScheduler::Deregister(GFX_STATIC::GFX_PA_TYPE& pass)
{
    AssertGraphics(m_RegisteredProducers.find(pass.GetName()) != m_RegisteredProducers.end(), "RenderPass not registered");
    m_RegisteredProducers.erase(pass.GetName());
}


void Ether::Graphics::FrameScheduler::PrecompilePipelineStates()
{
    ETH_MARKER_EVENT("Frame Scheduler - Precompile pipeline states");

    // Gather all PSOs declared in render passes
    // Compile what needs compiling (which should be everything)
    // Put it into resource context (unordered_map cache)

    for (auto iter = m_RegisteredProducers.begin(); iter != m_RegisteredProducers.end(); ++iter)
    {
        ETH_MARKER_EVENT((iter->second->GetName() + " - Initialize").c_str());
        iter->second->Initialize(m_ResourceContext);
    }
}

void Ether::Graphics::FrameScheduler::BuildSchedule()
{
    ETH_MARKER_EVENT("Frame Scheduler - Build Schedule");

    // TODO: Analyze all registered render passes
    //  - Figure out in what order the passes need to execute by creating a graph of dependencies
    //  - Figure out which passes can be executed in parallel (copy pipe, async compute pipe?)
    //  - Figure out resource lifetimes, and what can be aliased in a big placed resource
    //  - Batch resource barriers

    if (GraphicCore::GetGraphicConfig().GetUseShaderDaemon())
        m_ResourceContext.Reset();

    for (auto iter = m_RegisteredProducers.begin(); iter != m_RegisteredProducers.end(); ++iter)
        iter->second->Reset();

    ScheduleContext schedule;
    for (auto iter = m_RegisteredProducers.begin(); iter != m_RegisteredProducers.end(); ++iter)
    {
        ETH_MARKER_EVENT((iter->second->GetName() + " - GetInputOutput").c_str());
        iter->second->GetInputOutput(schedule, m_ResourceContext);
    }

    schedule.CreateResources(m_ResourceContext);

    // TODO: Run a topological sort to order the producers based on their inputs and outputs
    // defined in schedule context.
    // For now, manually specify order
    while (!m_OrderedProducers.empty())
        m_OrderedProducers.pop();

    m_OrderedProducers.push(ACCESS_GFX_PA(GlobalConstantsProducer).Get().get());
    m_OrderedProducers.push(ACCESS_GFX_PA(MaterialTableProducer).Get().get());
    m_OrderedProducers.push(ACCESS_GFX_PA(ProceduralSkyProducer).Get().get());
    m_OrderedProducers.push(ACCESS_GFX_PA(GBufferProducer).Get().get());
    m_OrderedProducers.push(ACCESS_GFX_PA(RaytracedLightingProducer).Get().get());
    m_OrderedProducers.push(ACCESS_GFX_PA(LightingCompositeProducer).Get().get());
    m_OrderedProducers.push(ACCESS_GFX_PA(PostFxSourceProducer).Get().get());
    m_OrderedProducers.push(ACCESS_GFX_PA(TemporalAAProducer).Get().get());
    m_OrderedProducers.push(ACCESS_GFX_PA(FinalCompositeProducer).Get().get());
}

void Ether::Graphics::FrameScheduler::RenderSingleThreaded(GraphicContext& context)
{
    ETH_MARKER_EVENT("Frame Scheduler - Render Single Threaded");

    context.Reset();
    GraphicDisplay& gfxDisplay = GraphicCore::GetGraphicDisplay();

    // For single threaded rendering, all producers will append into the same context
    while (!m_OrderedProducers.empty())
    {
        // This isEnabled check should be done during buildSchedule() instead.
        // Any disabled producers should just not participate in scheduling
        // (TODO)
        if (m_OrderedProducers.front()->IsEnabled())
        {
            ETH_MARKER_EVENT((m_OrderedProducers.front()->GetName() + " - Render").c_str());
            context.PushMarker(m_OrderedProducers.front()->GetName());
            m_OrderedProducers.front()->RenderFrame(context, m_ResourceContext);
            context.PopMarker();
        }

        m_OrderedProducers.pop();
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

