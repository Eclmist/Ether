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

#include "system/system.h"

#include "graphic/hal/dx12includes.h"
#include "graphic/gfx/virtualbuffer/gfxresourceallocation.h"
#include "graphic/gfx/virtualbuffer/gfxresourcememorypage.h"

/*
    This class is used to satisfy requests for memory that must be uploaded to the GPU.
    Each GfxLinearAllocator should only be associated to a single command list for simplicity.

    The GfxLinearAllocator can only be reset when all memory allocations are no longer
    "in-flight" on the command queue.

    This system is similar to the LinearAllocator in the official DirectX Sample MiniEngine.
*/
class GfxResourceAllocator
{
public:
    // Default page size of 2MB
    GfxResourceAllocator(
        wrl::ComPtr<ID3D12Device3> device,
        size_t pageSize = 2097152);

    ~GfxResourceAllocator();

    // Allocate memory in an upload heap. The allocation must not
    // exceed the size of a page.
    GfxResourceAllocation Allocate(size_t sizeInBytes, size_t alignment);

    // Release all allocated pages.
    void Reset();

public:
    inline size_t GetPageSize() const { return m_PageSize; };

private:
    std::shared_ptr<GfxResourceMemoryPage> RequestNewPage();

private:
    wrl::ComPtr<ID3D12Device3> m_Device;

    std::deque<std::shared_ptr<GfxResourceMemoryPage>> m_PagePool;
    std::deque<std::shared_ptr<GfxResourceMemoryPage>> m_AvailablePages;

    std::shared_ptr<GfxResourceMemoryPage> m_CurrentPage;

    size_t m_PageSize;
};
