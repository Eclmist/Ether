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
#include "graphics/schedule/framescheduler.h"
#include "graphics/schedule/schedulecontext.h"

#include "graphics/schedule/producers/gbufferproducer.h"
#include "graphics/schedule/producers/globalconstantsproducer.h"
#include "graphics/schedule/producers/lightingcompositeproducer.h"
#include "graphics/schedule/producers/materialtableproducer.h"
#include "graphics/schedule/producers/proceduralskyproducer.h"
#include "graphics/schedule/producers/raytracingresourceproducer.h"
#include "graphics/schedule/producers/raytracedlightingproducer.h"
#include "graphics/schedule/producers/raytracedtranslucencyproducer.h"
#include "graphics/schedule/producers/pathtracedlightingproducer.h"
#include "graphics/schedule/producers/translucencyproducer.h"

#include "graphics/schedule/producers/globalillumination/irradiancefieldproducer.h"

#include "graphics/schedule/producers/postprocess/postfxsourceproducer.h"
#include "graphics/schedule/producers/postprocess/bloomproducer.h"
#include "graphics/schedule/producers/postprocess/depthoffieldproducer.h"
#include "graphics/schedule/producers/postprocess/temporalaaproducer.h"
#include "graphics/schedule/producers/postprocess/finalcompositeproducer.h"

#if ETH_TOOLMODE
#include "graphics/schedule/producers/toolmode/editorgridproducer.h"
#include "graphics/schedule/producers/toolmode/editoroutlineproducer.h"
#endif

DECLARE_GFX_PA(GBufferProducer)
DECLARE_GFX_PA(GlobalConstantsProducer)
DECLARE_GFX_PA(MaterialTableProducer)
DECLARE_GFX_PA(ProceduralSkyProducer)
DECLARE_GFX_PA(RaytracingResourceProducer)
DECLARE_GFX_PA(IrradianceFieldProducer)
DECLARE_GFX_PA(RaytracedLightingProducer)
DECLARE_GFX_PA(RaytracedTranslucencyProducer)
DECLARE_GFX_PA(PathtracedLightingProducer)
DECLARE_GFX_PA(LightingCompositeProducer)
DECLARE_GFX_PA(TranslucencyProducer)

DECLARE_GFX_PA(PostFxSourceProducer)
DECLARE_GFX_PA(BloomProducer)
DECLARE_GFX_PA(DepthOfFieldProducer)
DECLARE_GFX_PA(TemporalAAProducer)
DECLARE_GFX_PA(FinalCompositeProducer)

DECLARE_GFX_PA(IrradianceFieldVisualizationProducer)

#if ETH_TOOLMODE
DECLARE_GFX_PA(EditorGridProducer)
DECLARE_GFX_PA(EditorOutlineProducer)
#endif

Ether::Graphics::FrameScheduler::FrameScheduler()
{
    Register(ACCESS_GFX_PA(GBufferProducer), new GBufferProducer());
    Register(ACCESS_GFX_PA(GlobalConstantsProducer), new GlobalConstantsProducer());
    Register(ACCESS_GFX_PA(MaterialTableProducer), new MaterialTableProducer());
    Register(ACCESS_GFX_PA(ProceduralSkyProducer), new ProceduralSkyProducer());
    Register(ACCESS_GFX_PA(RaytracingResourceProducer), new RaytracingResourceProducer());
    Register(ACCESS_GFX_PA(IrradianceFieldProducer), new IrradianceFieldProducer());
    Register(ACCESS_GFX_PA(RaytracedLightingProducer), new RaytracedLightingProducer());
    Register(ACCESS_GFX_PA(RaytracedTranslucencyProducer), new RaytracedTranslucencyProducer());
    Register(ACCESS_GFX_PA(PathtracedLightingProducer), new PathtracedLightingProducer());
    Register(ACCESS_GFX_PA(LightingCompositeProducer), new LightingCompositeProducer());
    Register(ACCESS_GFX_PA(TranslucencyProducer), new TranslucencyProducer());

    Register(ACCESS_GFX_PA(PostFxSourceProducer), new PostFxSourceProducer());
    Register(ACCESS_GFX_PA(BloomProducer), new BloomProducer());
    Register(ACCESS_GFX_PA(DepthOfFieldProducer), new DepthOfFieldProducer());
    Register(ACCESS_GFX_PA(TemporalAAProducer), new TemporalAAProducer());
    Register(ACCESS_GFX_PA(FinalCompositeProducer), new FinalCompositeProducer());

    Register(ACCESS_GFX_PA(IrradianceFieldVisualizationProducer), new IrradianceFieldVisualizationProducer());

#if ETH_TOOLMODE
    Register(ACCESS_GFX_PA(EditorGridProducer), new EditorGridProducer());
    Register(ACCESS_GFX_PA(EditorOutlineProducer), new EditorOutlineProducer());
#endif

    // Also for now, add imgui here
    m_ImguiWrapper = RhiImguiWrapper::InitForPlatform();
}

Ether::Graphics::FrameScheduler::~FrameScheduler()
{
    ACCESS_GFX_PA(GBufferProducer).Release();
    ACCESS_GFX_PA(GlobalConstantsProducer).Release();
    ACCESS_GFX_PA(LightingCompositeProducer).Release();
    ACCESS_GFX_PA(MaterialTableProducer).Release();
    ACCESS_GFX_PA(ProceduralSkyProducer).Release();
    ACCESS_GFX_PA(RaytracingResourceProducer).Release();
    ACCESS_GFX_PA(IrradianceFieldProducer).Release();
    ACCESS_GFX_PA(RaytracedLightingProducer).Release();
    ACCESS_GFX_PA(PathtracedLightingProducer).Release();
    ACCESS_GFX_PA(TranslucencyProducer).Release();
    ACCESS_GFX_PA(RaytracedTranslucencyProducer).Release();

    ACCESS_GFX_PA(PostFxSourceProducer).Release();
    ACCESS_GFX_PA(BloomProducer).Release();
    ACCESS_GFX_PA(DepthOfFieldProducer).Release();
    ACCESS_GFX_PA(TemporalAAProducer).Release();
    ACCESS_GFX_PA(FinalCompositeProducer).Release();

    ACCESS_GFX_PA(IrradianceFieldVisualizationProducer).Release();

#if ETH_TOOLMODE
    ACCESS_GFX_PA(EditorGridProducer).Release();
    ACCESS_GFX_PA(EditorOutlineProducer).Release();
#endif
}

void Ether::Graphics::FrameScheduler::Register(GFX_STATIC::GFX_PA_TYPE& pass, GraphicProducer* producer)
{
    AssertGraphics(!m_RegisteredProducers.contains(pass.GetName()), "Producer already registered");
    AssertGraphics(producer != nullptr, "Cannot register null producer");
    m_RegisteredProducers.emplace(pass.GetName(), producer);
    pass.Create(producer);
}

void Ether::Graphics::FrameScheduler::Deregister(GFX_STATIC::GFX_PA_TYPE& pass)
{
    AssertGraphics(m_RegisteredProducers.contains(pass.GetName()), "Producer not registered");
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

    if (GraphicCore::GetGraphicConfig().UseShaderDaemon())
        m_ResourceContext.Reset();

    for (auto iter = m_RegisteredProducers.begin(); iter != m_RegisteredProducers.end(); ++iter)
        iter->second->Reset();

    ScheduleContext schedule;
    for (auto iter = m_RegisteredProducers.begin(); iter != m_RegisteredProducers.end(); ++iter)
    {
        ETH_MARKER_EVENT(iter->second->GetName().c_str());
        iter->second->GetInputOutput(schedule, m_ResourceContext);
    }

    schedule.CreateResources(m_ResourceContext);

    // TODO: Run a topological sort to order the producers based on their inputs and outputs
    // defined in schedule context.
    // For now, manually specify order
    while (!m_OrderedProducers.empty())
        m_OrderedProducers.pop();

    m_OrderedProducers.push(ACCESS_GFX_PA(GlobalConstantsProducer).Get());
    m_OrderedProducers.push(ACCESS_GFX_PA(MaterialTableProducer).Get());
    m_OrderedProducers.push(ACCESS_GFX_PA(ProceduralSkyProducer).Get());
    m_OrderedProducers.push(ACCESS_GFX_PA(RaytracingResourceProducer).Get());
    m_OrderedProducers.push(ACCESS_GFX_PA(IrradianceFieldProducer).Get());
    m_OrderedProducers.push(ACCESS_GFX_PA(GBufferProducer).Get());
    m_OrderedProducers.push(ACCESS_GFX_PA(RaytracedLightingProducer).Get());
    m_OrderedProducers.push(ACCESS_GFX_PA(PathtracedLightingProducer).Get());
    m_OrderedProducers.push(ACCESS_GFX_PA(LightingCompositeProducer).Get());
    m_OrderedProducers.push(ACCESS_GFX_PA(TranslucencyProducer).Get());
    m_OrderedProducers.push(ACCESS_GFX_PA(RaytracedTranslucencyProducer).Get());

    // Order of post process is important, obviously
    // Reference: https://www.renderingevolution.net/?p=103
    m_OrderedProducers.push(ACCESS_GFX_PA(PostFxSourceProducer).Get());
    m_OrderedProducers.push(ACCESS_GFX_PA(DepthOfFieldProducer).Get());
    m_OrderedProducers.push(ACCESS_GFX_PA(BloomProducer).Get());
    m_OrderedProducers.push(ACCESS_GFX_PA(TemporalAAProducer).Get());
    // TODO: Add a tonemapping pass instead of dumping it in final composite, and move it before TAA
    m_OrderedProducers.push(ACCESS_GFX_PA(FinalCompositeProducer).Get());

    // Debug Visualizers:
    m_OrderedProducers.push(ACCESS_GFX_PA(IrradianceFieldVisualizationProducer).Get());

#if ETH_TOOLMODE
    m_OrderedProducers.push(ACCESS_GFX_PA(EditorGridProducer).Get());
    m_OrderedProducers.push(ACCESS_GFX_PA(EditorOutlineProducer).Get());
#endif

}

void Ether::Graphics::FrameScheduler::RenderSingleThreaded(GraphicContext& gfxContext)
{
    ETH_MARKER_EVENT("Frame Scheduler - Render Single Threaded");

    gfxContext.Reset();
    gfxContext.SetResourceContext(m_ResourceContext);

    GraphicDisplay& gfxDisplay = GraphicCore::GetGraphicDisplay();

    // For single threaded rendering, all producers will append into the same context
    while (!m_OrderedProducers.empty())
    {
        // This isEnabled check should be done during buildSchedule() instead.
        // Any disabled producers should just not participate in scheduling
        // (TODO)
        if (m_OrderedProducers.front()->IsEnabled())
        {
            gfxContext.PushMarker(m_OrderedProducers.front()->GetName());
            m_OrderedProducers.front()->RenderFrame(gfxContext, m_ResourceContext);
            gfxContext.PopMarker();
        }

        m_OrderedProducers.pop();
    }

    gfxContext.FinalizeAndExecute();

    m_ImguiWrapper->Render();

    gfxContext.Reset();
    gfxContext.TransitionResource(gfxDisplay.GetBackBuffer(), RhiResourceState::Present);
    gfxContext.FinalizeAndExecute();
}

void Ether::Graphics::FrameScheduler::RenderMultiThreaded(GraphicContext& context)
{
}

