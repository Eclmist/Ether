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

#include "graphic/hal/dx12descriptorheap.h"
#include "graphic/gfx/virtualbuffer/gfxdescriptorallocation.h"
#include "graphic/gfx/virtualbuffer/gfxdescriptormemoryblock.h"
#include "graphic/gfx/virtualbuffer/gfxstaledescriptorinfo.h"

class GfxDescriptorMemoryPage
{
public:
    GfxDescriptorMemoryPage(
        wrl::ComPtr<ID3D12Device3> device,
        D3D12_DESCRIPTOR_HEAP_TYPE type,
        uint32_t numDescriptors
    );

    //! @brief Checks if the page has enough memory to satisfy an allocation request
    bool HasSpace(uint32_t numDescriptors) const;

    GfxDescriptorAllocation Allocate(uint32_t numDescriptors);
    void Free(GfxDescriptorAllocation&& allocation, uint32_t frameNumber);
    void ReleaseStaleDescriptors(uint64_t graphicFrameNumber);

public:
    inline D3D12_DESCRIPTOR_HEAP_TYPE GetHeapType() const { return m_DescriptorHeap->GetType(); };
    inline uint32_t GetNumDescriptorsInHeap() const { return m_DescriptorHeap->GetNumDescriptors(); };
    inline uint32_t GetNumFreeHandles() const { return m_NumFreeHandles; };

protected:
    void AddNewBlock(uint32_t offset, uint32_t numDescriptors);
    void FreeBlock(uint32_t offset, uint32_t numDescriptors);

private:
    std::unique_ptr<DX12DescriptorHeap> m_DescriptorHeap;

    std::map<uint32_t, GfxDescriptorMemoryBlock> m_FreeListByOffset;
    std::multimap<uint32_t, std::map<uint32_t, GfxDescriptorMemoryBlock>::iterator> m_FreeListBySize;
    std::queue<GfxStaleDescriptorInfo> m_StaleDescriptors;
    std::mutex m_AllocationMutex;

    uint32_t m_NumFreeHandles;
};
