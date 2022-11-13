/*
    This file is part of Ether, an open-source DirectX 12 renderer.

    Copyright (c) 2020-2023 Samuel Huang - All rights reserved.

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

#include "common/memory/freelistallocator.h"
#include "graphic/rhi/rhidescriptorheap.h"
#include "descriptorallocation.h"

ETH_NAMESPACE_BEGIN

constexpr uint32_t DefaultMaxHeapSize = 262144; // 2^18, but can theoretically be infinite

class DescriptorAllocator
{
public:
    DescriptorAllocator(
        RhiDescriptorHeapType type,
        bool isShaderVisible = false,
        uint32_t maxHeapSize = DefaultMaxHeapSize
    );

    ~DescriptorAllocator() = default;

public:
    inline RhiDescriptorHeapHandle GetDescriptorHeap() const { return m_DescriptorHeap; }
    inline bool IsShaderVisible() const { return m_IsShaderVisible; }

public:
    std::shared_ptr<DescriptorAllocation> Allocate(uint32_t numDescriptors = 1);
    void Free(const DescriptorAllocation& allocation);
    
private:
    bool ReclaimStaleAllocations(uint32_t numIndices);

private:
    struct StaleAllocation
    {
        StaleAllocation(uint32_t offset, uint32_t numDescriptors)
            : m_IndexInHeap(offset)
            , m_NumDescriptors(numDescriptors) {}

        uint32_t m_IndexInHeap;
        uint32_t m_NumDescriptors;
    };

    uint32_t m_MaxDescriptors;
    bool m_IsShaderVisible;

    RhiDescriptorHeapType m_HeapType;
    RhiDescriptorHeapHandle m_DescriptorHeap;
    FreeListAllocator m_FreeListAllocator;

    std::queue<StaleAllocation> m_StaleAllocations;
};

ETH_NAMESPACE_END

