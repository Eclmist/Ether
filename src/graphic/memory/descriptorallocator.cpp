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

#include "descriptorallocator.h"

ETH_NAMESPACE_BEGIN

//DescriptorAllocator::DescriptorAllocator(RHIDescriptorHeapType type, uint32_t numDescriptorsPerHeap)
//    : m_HeapType(type)
//    , m_NumDescriptorsPerHeap(numDescriptorsPerHeap)
//{
//}
//
//std::shared_ptr<DescriptorAllocatorPage> DescriptorAllocator::CreateAllocatorPage()
//{
//    auto newPage = std::make_shared<DescriptorAllocatorPage>(m_HeapType, m_NumDescriptorsPerHeap);
//
//    m_HeapPool.emplace_back(newPage);
//    m_AvailableHeaps.insert(m_HeapPool.size() - 1);
//
//    return newPage;
//}
//
//DescriptorAllocation DescriptorAllocator::Allocate(uint32_t numDescriptors)
//{
//    std::lock_guard<std::mutex> lock(m_AllocationMutex);
//
//    DescriptorAllocation allocation;
//
//    for (auto iter = m_AvailableHeaps.begin(); iter != m_AvailableHeaps.end(); ++iter)
//    {
//        auto allocatorPage = m_HeapPool[*iter];
//
//        allocation = allocatorPage->Allocate(numDescriptors);
//
//        if (allocatorPage->NumFreeHandles() == 0)
//        {
//            iter = m_AvailableHeaps.erase(iter);
//        }
//
//        // A valid allocation has been found.
//        if (!allocation.IsNull())
//        {
//            break;
//        }
//
//        // No available heap could satisfy the requested number of descriptors.
//        if (allocation.IsNull())
//        {
//            m_NumDescriptorsPerHeap = std::max(m_NumDescriptorsPerHeap, numDescriptors);
//            auto newPage = CreateAllocatorPage();
//
//            allocation = newPage->Allocate(numDescriptors);
//        }
//
//        return allocation;
//    }
//}
//
//void DescriptorAllocator::ReleaseStaleDescriptors(uint64_t frameNumber)
//{
//    std::lock_guard<std::mutex> lock(m_AllocationMutex);
//
//    for (size_t i = 0; i < m_HeapPool.size(); ++i)
//    {
//        auto page = m_HeapPool[i];
//
//        page->ReleaseStaleDescriptors(frameNumber);
//
//        if (page->NumFreeHandles() > 0)
//        {
//            m_AvailableHeaps.insert(i);
//        }
//    }
//}



ETH_NAMESPACE_END

