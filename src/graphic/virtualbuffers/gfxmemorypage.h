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
#include "graphic/hal/resource/dx12committedresource.h"
#include "graphic/virtualbuffers/gfxallocation.h"

/**
 * WARNING: This memory page is *not* thread safe! It is not designed to be used across threads
 * to avoid unnecessary overhead of locking/unlocking mutexes. 
 */
class GfxMemoryPage
{
public:
    GfxMemoryPage(
        wrl::ComPtr<ID3D12Device3> device,
        size_t sizeInBytes
    );

    ~GfxMemoryPage();

public:
    // Check to see if the page has room to allocate.
    bool HasSpace(size_t sizeInBytes, size_t alignment) const;

    // Allocate memory from the page.
    GfxAllocation Allocate(size_t sizeInBytes, size_t alignment);

    // Reset the page for reuse.
    void Reset();

private:
    std::unique_ptr<DX12CommittedResource> m_Resource;

    void* m_CPUMemoryPtr;
    D3D12_GPU_VIRTUAL_ADDRESS m_GPUMemoryPtr;

    size_t m_PageSize;
    size_t m_Offset;
};
