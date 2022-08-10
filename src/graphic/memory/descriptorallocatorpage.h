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

#include "descriptorallocation.h"
#include "common/memory/freelistallocator.h"

ETH_NAMESPACE_BEGIN

class DescriptorAllocatorPage
{
public:
    DescriptorAllocatorPage(RhiDescriptorHeapType type, uint32_t numDescriptors);
    ~DescriptorAllocatorPage() = default;

public:
    inline RhiDescriptorHeapType GetHeapType() const { return m_HeapType; }
    inline uint32_t GetNumFreeHandles() const { return m_NumFreeHandles; }
    inline bool HasSpace(uint32_t numDescriptors) const { return m_FreeListAllocator.HasSpace(numDescriptors); }

public:
    std::shared_ptr<DescriptorAllocation> Allocate(uint32_t numDescriptors);
    void Free(DescriptorAllocation&& allocation, uint64_t frameNumber);
    void ReleaseStaleDescriptors(uint64_t frameNumber);
    uint32_t GetOffset(RhiCpuHandle descriptor);

private:
    std::mutex m_AllocationMutex;

    RhiDescriptorHeapType m_HeapType;
    uint32_t m_MaxDescriptors;
    uint32_t m_NumFreeHandles;

    FreeListAllocator m_FreeListAllocator;
    RhiDescriptorHeapHandle m_DescriptorHeap;

    std::queue<std::pair<DescriptorAllocation, uint64_t>> m_StaleDescriptors;
};

ETH_NAMESPACE_END

