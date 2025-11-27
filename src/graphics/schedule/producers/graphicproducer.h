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

#pragma once

#include "graphics/pch.h"
#include "graphics/schedule/frameschedulerutils.h"
#include "graphics/schedule/schedulecontext.h"
#include "graphics/context/resourcecontext.h"
#include "graphics/context/graphiccontext.h"
#include "graphics/rhi/rhiresourceviews.h"
#include "graphics/rhi/rhirootsignaturebindingtable.h"

namespace Ether::Graphics
{
class GraphicProducer
{
public:
    GraphicProducer(const char* name = "Unnamed Producer");
    virtual ~GraphicProducer() = default;

public:
    inline const std::string& GetName() const { return m_Name; };

public:
    virtual void Initialize(ResourceContext& rc) = 0;
    virtual void GetInputOutput(ScheduleContext& schedule, ResourceContext& rc) = 0;
    virtual void RenderFrame(GraphicContext& ctx, ResourceContext& rc) = 0;

protected:
    friend class FrameScheduler;
    virtual void Reset();
    virtual bool IsEnabled();

protected:
    UploadBufferAllocator& GetFrameAllocator();
    uint64_t GetRingBufferOffset() const;

protected:
    std::string m_Name;
    std::unique_ptr<RhiRootSignature> m_RootSignature;

private:
    std::unique_ptr<UploadBufferAllocator> m_FrameLocalUploadBuffer[MaxSwapChainBuffers];
};
}