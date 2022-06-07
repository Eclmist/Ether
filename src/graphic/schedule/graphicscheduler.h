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

#pragma once

#include "graphic/schedule/renderpass/renderpass.h"
#include "graphic/schedule/graphicresourceid.h"

ETH_NAMESPACE_BEGIN

class GraphicScheduler : public NonCopyable
{
public:
    GraphicScheduler() = default;
    ~GraphicScheduler() = default;

    void RegisterRenderPasses();
    void ScheduleRenderPasses(GraphicContext& context, ResourceContext& rc);
    void RenderPasses(GraphicContext& context, ResourceContext& rc);

    void ScheduleTextureResize(uint32_t width, uint32_t height);

private:
    std::vector<RenderPass*> m_RegisteredRenderPasses;
    std::vector<RenderPass*> m_OrderedRenderPasses;

    std::unordered_map<std::string, RhiResourceHandle> m_Resources;

    std::vector<RhiResourceHandle> m_ResizeQueue;
};

ETH_NAMESPACE_END

