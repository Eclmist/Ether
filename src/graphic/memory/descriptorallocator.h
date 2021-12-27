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

#include "descriptorallocation.h"

ETH_NAMESPACE_BEGIN

//class DescriptorAllocatorPage;
//
//class DescriptorAllocator
//{
//public:
//    DescriptorAllocator(RHIDescriptorHeapType type, uint32_t numDescriptorsPerHeap = 256);
//    virtual ~DescriptorAllocator();
//
//    DescriptorAllocation Allocate(uint32_t numDescriptors = 1);
//    void ReleaseStaleDescriptors(uint64_t frameNumber);
//
//private:
//    using DescriptorHeapPool = std::vector<std::shared_ptr<DescriptorAllocatorPage>>;
//
//    // Create a new heap with a specific number of descriptors.
//    std::shared_ptr<DescriptorAllocatorPage> CreateAllocatorPage();
//
//    RHIDescriptorHeapType m_HeapType;
//    uint32_t m_NumDescriptorsPerHeap;
//
//    DescriptorHeapPool m_HeapPool;
//    // Indices of available heaps in the heap pool.
//    std::set<size_t> m_AvailableHeaps;
//
//    std::mutex m_AllocationMutex;
//};

ETH_NAMESPACE_END

