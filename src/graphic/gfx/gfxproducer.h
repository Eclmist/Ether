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

#pragma once

#include "graphic/hal/dx12commandlist.h"
#include "graphic/hal/dx12commandallocator.h"
#include "graphic/hal/dx12commandqueue.h"
#include "graphic/hal/dx12pipelinestate.h"

#include "graphic/gfx/gfxcontext.h"
#include "graphic/gfx/gfxschedulecontext.h"

class GfxProducer : NonCopyable
{
public:
    GfxProducer(const char* name);
    ~GfxProducer() = default;

    virtual bool GetInputOutput(GfxScheduleContext& scheduleContext);
    //virtual void BuildCommandLists() = 0;
    //virtual void Reset() = 0;
    //virtual void Flush() = 0;

protected:
    const char* m_Name;
    const GfxContext* m_Context;

    std::unique_ptr<DX12PipelineState> m_PipelineState;
};
