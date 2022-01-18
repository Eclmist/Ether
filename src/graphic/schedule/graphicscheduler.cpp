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

#include "graphicscheduler.h"
#include "graphic/schedule/renderpass/clearframebufferpass.h"
#include "graphic/schedule/renderpass/deferredlightingpass.h"
#include "graphic/schedule/renderpass/gbufferpass.h"
#include "graphic/schedule/renderpass/hardcodedrenderpass.h"

ETH_NAMESPACE_BEGIN

DECLARE_GFX_PASS(ClearFrameBufferPass);
DECLARE_GFX_PASS(HardCodedRenderPass);
DECLARE_GFX_PASS(GBufferPass);
DECLARE_GFX_PASS(DeferredLightingPass);

void GraphicScheduler::RegisterRenderPasses()
{
    m_RegisteredRenderPasses.push_back(&GFX_PASS(ClearFrameBufferPass));
    //m_RegisteredRenderPasses.push_back(&GFX_PASS(HardCodedRenderPass));
    m_RegisteredRenderPasses.push_back(&GFX_PASS(GBufferPass));
    m_RegisteredRenderPasses.push_back(&GFX_PASS(DeferredLightingPass));
}

void GraphicScheduler::ScheduleRenderPasses(GraphicContext& context, ResourceContext& rc)
{
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

    for (int i = 0; i < m_OrderedRenderPasses.size(); ++i)
        m_OrderedRenderPasses[i]->Render(context, rc);

    m_OrderedRenderPasses.clear();
}

ETH_NAMESPACE_END

