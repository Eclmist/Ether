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

ETH_NAMESPACE_BEGIN

//class DescriptorAllocatorPage
//{
//public:
//    DescriptorAllocatorPage(RHIDescriptorHeapType type, uint32_t numDescriptors);
//
//    RHIDescriptorHeapType GetHeapType() const;
//
//    /**
//     * Check to see if this descriptor page has a contiguous block of descriptors
//     * large enough to satisfy the request.
//     */
//    bool HasSpace(uint32_t numDescriptors) const;
//
//    /**
//	 * Get the number of available handles in the heap.
//	 */
//    uint32_t NumFreeHandles() const;
//
//    /**
//     * Allocate a number of descriptors from this descriptor heap.
//     * If the allocation cannot be satisfied, then a NULL descriptor
//     * is returned.
//     */
//    DescriptorAllocation Allocate(uint32_t numDescriptors);
//
//    /**
//	 * Return a descriptor back to the heap.
//	 * @param frameNumber Stale descriptors are not freed directly, but put
//	 * on a stale allocations queue. Stale allocations are returned to the heap
//	 * using the DescriptorAllocatorPage::ReleaseStaleAllocations method.
//	 */
//    void Free(DescriptorAllocation&& descriptorHandle, uint64_t frameNumber);
//
//    /**
//     * Returned the stale descriptors back to the descriptor heap.
//     */
//    void ReleaseStaleDescriptors(uint64_t frameNumber);
//
//protected:
//
//    // Compute the offset of the descriptor handle from the start of the heap.
//    uint32_t ComputeOffset(D3D12_CPU_DESCRIPTOR_HANDLE handle);
//
//    // Adds a new block to the free list.
//    void AddNewBlock(uint32_t offset, uint32_t numDescriptors);
//
//    // Free a block of descriptors.
//    // This will also merge free blocks in the free list to form larger blocks
//    // that can be reused.
//    void FreeBlock(uint32_t offset, uint32_t numDescriptors);
//
//private:
//    // The offset (in descriptors) within the descriptor heap.
//    using OffsetType = uint32_t;
//    // The number of descriptors that are available.
//    using SizeType = uint32_t;
//
//    struct FreeBlockInfo;
//    // A map that lists the free blocks by the offset within the descriptor heap.
//    using FreeListByOffset = std::map<OffsetType, FreeBlockInfo>;
//
//    // A map that lists the free blocks by size.
//    // Needs to be a multimap since multiple blocks can have the same size.
//    using FreeListBySize = std::multimap<SizeType, FreeListByOffset::iterator>;
//
//    struct FreeBlockInfo
//    {
//        FreeBlockInfo(SizeType size)
//            : Size(size)
//        {}
//
//        SizeType Size;
//        FreeListBySize::iterator FreeListBySizeIt;
//    };
//
//    struct StaleDescriptorInfo
//    {
//        StaleDescriptorInfo(OffsetType offset, SizeType size, uint64_t frame)
//            : Offset(offset)
//            , Size(size)
//            , FrameNumber(frame)
//        {}
//
//        // The offset within the descriptor heap.
//        OffsetType Offset;
//        // The number of descriptors
//        SizeType Size;
//        // The frame number that the descriptor was freed.
//        uint64_t FrameNumber;
//    };
//
//    // Stale descriptors are queued for release until the frame that they were freed
//	// has completed.
//    using StaleDescriptorQueue = std::queue<StaleDescriptorInfo>;
//
//    FreeListByOffset m_FreeListByOffset;
//    FreeListBySize m_FreeListBySize;
//    StaleDescriptorQueue m_StaleDescriptors;
//
//    RHIDescriptorHeapHandle m_DescriptorHeap;
//    RHIDescriptorHeapType m_HeapType;
//    RHIResourceHandle m_BaseDescriptor;
//
//    uint32_t m_DescriptorHandleIncrementSize;
//    uint32_t m_NumDescriptorsInHeap;
//    uint32_t m_NumFreeHandles;
//
//    std::mutex m_AllocationMutex;
//};

ETH_NAMESPACE_END

