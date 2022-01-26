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

#include "graphicscheduler.h"
#include "graphic/schedule/renderpass/clearframebufferpass.h"
#include "graphic/schedule/renderpass/commonconstantsuploadpass.h"
#include "graphic/schedule/renderpass/deferredlightingpass.h"
#include "graphic/schedule/renderpass/gbufferpass.h"
#include "graphic/schedule/renderpass/proceduralskypass.h"

#ifdef ETH_TOOLMODE
#include "graphic/schedule/renderpass/texturedebugpass.h"
#endif

ETH_NAMESPACE_BEGIN

DECLARE_GFX_PASS(ClearFrameBufferPass);
DECLARE_GFX_PASS(CommonConstantsUploadPass);
DECLARE_GFX_PASS(DeferredLightingPass);
DECLARE_GFX_PASS(GBufferPass);
DECLARE_GFX_PASS(ProceduralSkyPass);

#ifdef ETH_TOOLMODE
DECLARE_GFX_PASS(TextureDebugPass);
#endif

void GraphicScheduler::RegisterRenderPasses()
{
    m_RegisteredRenderPasses.push_back(&GFX_PASS(CommonConstantsUploadPass));
    m_RegisteredRenderPasses.push_back(&GFX_PASS(ClearFrameBufferPass));
    m_RegisteredRenderPasses.push_back(&GFX_PASS(ProceduralSkyPass));
    m_RegisteredRenderPasses.push_back(&GFX_PASS(GBufferPass));
    m_RegisteredRenderPasses.push_back(&GFX_PASS(DeferredLightingPass));
    ETH_TOOLONLY(m_RegisteredRenderPasses.push_back(&GFX_PASS(TextureDebugPass)));

    for (auto renderPass : m_RegisteredRenderPasses)
        renderPass->Initialize();
}

void GraphicScheduler::ScheduleRenderPasses(GraphicContext& context, ResourceContext& rc)
{
    OPTICK_EVENT("Graphic Scheduler - Schedule");

    for (auto renderPass : m_RegisteredRenderPasses)
        renderPass->RegisterInputOutput(context, rc);

    // [ETH-3] No scheduling yet, as a minimum POC
    for (int i = 0; i < m_RegisteredRenderPasses.size(); ++i)
        m_OrderedRenderPasses.push_back(m_RegisteredRenderPasses[i]);
}

void GraphicScheduler::RenderPasses(GraphicContext& context, ResourceContext& rc)
{
    //while (!m_ResizeQueue.empty())
    //{
        //m_ResizeQueue.front()->Resize();
        //m_ResizeQueue.pop();
    //}
    OPTICK_EVENT("Graphic Scheduler - RenderPasses");

    for (int i = 0; i < m_OrderedRenderPasses.size(); ++i)
        m_OrderedRenderPasses[i]->Render(context, rc);

    m_OrderedRenderPasses.clear();
}

ETH_NAMESPACE_END

