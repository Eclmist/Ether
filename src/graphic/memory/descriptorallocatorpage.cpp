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

#include "descriptorallocatorpage.h"
#include "graphic/rhi/rhidevice.h"
#include "graphic/rhi/rhidescriptorheap.h"

ETH_NAMESPACE_BEGIN

DescriptorAllocatorPage::DescriptorAllocatorPage(RhiDescriptorHeapType type, uint32_t numDescriptors)
    : m_HeapType(type)
    , m_MaxDescriptors(numDescriptors)
    , m_NumFreeHandles(numDescriptors)
{
    GraphicCore::GetDevice()->CreateDescriptorHeap({
        type,
        RhiDescriptorHeapFlag::None,
        numDescriptors
    }, m_DescriptorHeap);

    m_FreeListAllocator.AddBlock(0, m_MaxDescriptors);
}

std::shared_ptr<DescriptorAllocation> DescriptorAllocatorPage::Allocate(uint32_t numDescriptors)
{
    std::lock_guard<std::mutex> guard(m_AllocationMutex);

    if (!HasSpace(numDescriptors))
    {
        LogGraphicsError("An attempt was made to allocate more descriptors than a descriptor allocator page allows");
        throw std::bad_alloc();
    }

    FreeListAllocation alloc = m_FreeListAllocator.Allocate(numDescriptors);
    uint32_t descriptorSize = m_DescriptorHeap->GetHandleIncrementSize();
    RhiCpuHandle allocBaseHandle = { m_DescriptorHeap->GetBaseCpuHandle().m_Ptr + alloc.m_Offset * descriptorSize };

    return std::make_shared<DescriptorAllocation>(
        allocBaseHandle,
        alloc.m_Size,
        alloc.m_Offset,
        descriptorSize,
        this);
}

void DescriptorAllocatorPage::Free(DescriptorAllocation&& allocation, uint64_t frameNumber)
{
    std::lock_guard<std::mutex> guard(m_AllocationMutex);

    uint32_t offset = GetOffset(allocation.GetDescriptorHandle());
    m_StaleDescriptors.emplace(allocation, frameNumber);
}

void DescriptorAllocatorPage::ReleaseStaleDescriptors(uint64_t frameNumber)
{
    std::lock_guard<std::mutex> guard(m_AllocationMutex);

    while (!m_StaleDescriptors.empty() && m_StaleDescriptors.front().second <= frameNumber)
    {
        DescriptorAllocation& staleDescriptor = m_StaleDescriptors.front().first;
        uint32_t offset = staleDescriptor.GetPageOffset();
        uint32_t numDescriptors = staleDescriptor.GetNumDescriptors();
        m_FreeListAllocator.FreeBlock(offset, numDescriptors);
        m_StaleDescriptors.pop();
    }
}

uint32_t DescriptorAllocatorPage::GetOffset(RhiCpuHandle descriptor)
{
    return descriptor.m_Ptr - m_DescriptorHeap->GetBaseCpuHandle().m_Ptr;
}

ETH_NAMESPACE_END

