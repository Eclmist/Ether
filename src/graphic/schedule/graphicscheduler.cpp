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
#include "graphic/schedule/renderpass/hardcodedrenderpass.h"

ETH_NAMESPACE_BEGIN

namespace GraphicLinkSpace
{
    HardCodedRenderPass g_HardCodedRenderPass("Test Render Pass");
}

void GraphicScheduler::BuildSchedule()
{
    // [ETH-3] No scheduling yet, as a minimum POC
    for (int i = 0; i < m_RegisteredRenderPasses.size(); ++i)
        m_OrderedRenderPasses.push_back(m_RegisteredRenderPasses[i]);
}

void GraphicScheduler::ScheduleRenderPasses()
{
    m_RegisteredRenderPasses.push_back((RenderPass*)&GraphicLinkSpace::g_HardCodedRenderPass);
}

void GraphicScheduler::Render(GraphicContext& context)
{
    for (int i = 0; i < m_OrderedRenderPasses.size(); ++i)
        m_OrderedRenderPasses[i]->Render(context);
}

ETH_NAMESPACE_END

