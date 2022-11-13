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

#include "graphicscheduler.h"

#include "graphic/schedule/producers/depthstencilproducer.h"

#include "graphic/schedule/renderpass/clearframebufferpass.h"
#include "graphic/schedule/renderpass/globalconstantbufferproducer.h"
#include "graphic/schedule/renderpass/deferredlightingproducer.h"
#include "graphic/schedule/renderpass/gbufferproducer.h"
//#include "graphic/schedule/renderpass/proceduralskypass.h"
//#include "graphic/schedule/renderpass/bloompass.h"

#ifdef ETH_TOOLMODE
#include "graphic/schedule/renderpass/editorgizmospass.h"
//#include "graphic/schedule/renderpass/texturedebugpass.h"
#endif

#include "graphic/schedule/renderpass/debugpass.h"

ETH_NAMESPACE_BEGIN

//DECLARE_GFX_PASS(BloomPass);
DECLARE_GFX_PASS(ClearFrameBufferPass);
DECLARE_GFX_PASS(GlobalConstantBufferProducer);
DECLARE_GFX_PASS(DeferredLightingProducer);
DECLARE_GFX_PASS(GBufferProducer);
//DECLARE_GFX_PASS(ProceduralSkyPass);

DECLARE_GFX_PASS(DepthStencilProducer);
DECLARE_GFX_PASS(DebugPass);

#ifdef ETH_TOOLMODE
DECLARE_GFX_PASS(EditorGizmosPass);
//DECLARE_GFX_PASS(TextureDebugPass);
#endif

void GraphicScheduler::RegisterRenderPasses()
{
    m_RegisteredRenderPasses.push_back(&GFX_PASS(GlobalConstantBufferProducer));
    m_RegisteredRenderPasses.push_back(&GFX_PASS(DepthStencilProducer));
    m_RegisteredRenderPasses.push_back(&GFX_PASS(ClearFrameBufferPass));
    //m_RegisteredRenderPasses.push_back(&GFX_PASS(ProceduralSkyPass));
    m_RegisteredRenderPasses.push_back(&GFX_PASS(GBufferProducer));
    m_RegisteredRenderPasses.push_back(&GFX_PASS(DeferredLightingProducer));
    //m_RegisteredRenderPasses.push_back(&GFX_PASS(BloomPass));
    ETH_TOOLONLY(m_RegisteredRenderPasses.push_back(&GFX_PASS(EditorGizmosPass)));
    //ETH_TOOLONLY(m_RegisteredRenderPasses.push_back(&GFX_PASS(TextureDebugPass)));

    //m_RegisteredRenderPasses.push_back(&GFX_PASS(DebugPass));

    for (auto renderPass : m_RegisteredRenderPasses)
        renderPass->Initialize();
}

void GraphicScheduler::ScheduleRenderPasses(GraphicContext& context, ResourceContext& rc)
{
    ETH_MARKER_EVENT("Graphic Scheduler - Schedule");

    for (auto renderPass : m_RegisteredRenderPasses)
        renderPass->RegisterInputOutput(context, rc);

    // [ETH-3] No scheduling yet, as a minimum POC
    for (int i = 0; i < m_RegisteredRenderPasses.size(); ++i)
        m_OrderedRenderPasses.push_back(m_RegisteredRenderPasses[i]);
}

void GraphicScheduler::RenderPasses(GraphicContext& context, ResourceContext& rc)
{
    for (int i = 0; i < m_OrderedRenderPasses.size(); ++i)
    {
        ETH_MARKER_EVENT(m_OrderedRenderPasses[i]->GetName().c_str());
        context.PushMarker(m_OrderedRenderPasses[i]->GetName().c_str());
        m_OrderedRenderPasses[i]->Render(context, rc);
        context.PopMarker();
    }

    m_OrderedRenderPasses.clear();
}

ETH_NAMESPACE_END

