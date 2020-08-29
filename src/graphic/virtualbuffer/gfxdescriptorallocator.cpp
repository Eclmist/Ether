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

#include "gfxdescriptorallocator.h"

GfxDescriptorAllocator::GfxDescriptorAllocator(
    wrl::ComPtr<ID3D12Device3> device,
    D3D12_DESCRIPTOR_HEAP_TYPE type,
    uint32_t numDescriptorsPerHeap)
    : m_Device(device)
    , m_HeapType(type)
    , m_NumDescriptorsPerHeap(numDescriptorsPerHeap)
{
}

GfxDescriptorAllocator::~GfxDescriptorAllocator()
{

}

GfxDescriptorAllocation GfxDescriptorAllocator::Allocate(uint32_t numDescriptors)
{
    std::lock_guard<std::mutex> lock(m_AllocationMutex);

    GfxDescriptorAllocation allocation;

    for (auto iter = m_AvailableHeaps.begin(); iter != m_AvailableHeaps.end(); ++iter)
    {
        std::shared_ptr<GfxDescriptorMemoryPage> page = m_HeapPool[*iter];
        allocation = page->Allocate(numDescriptors);

        if (page->GetNumFreeHandles() == 0)
        {
            iter = m_AvailableHeaps.erase(iter);
        }

        // break out of the loop once a we are able to successfully allocate
        if (!allocation.IsNull())
        {
            break;
        }
    }

    // No heap were able to allocated
    if (allocation.IsNull())
    {
        m_NumDescriptorsPerHeap = std::max(m_NumDescriptorsPerHeap, numDescriptors);
        std::shared_ptr<GfxDescriptorMemoryPage> newPage = CreateAllocatorPage();
        allocation = newPage->Allocate(numDescriptors);
    }

    return allocation;
}

void GfxDescriptorAllocator::ReleaseStaleDescriptors(uint64_t frameNumber)
{
    std::lock_guard<std::mutex> lock(m_AllocationMutex);

    for (int i = 0; i < m_HeapPool.size(); ++i)
    {
        std::shared_ptr<GfxDescriptorMemoryPage> page = m_HeapPool[i];
        page->ReleaseStaleDescriptors(frameNumber);

        if (page->GetNumFreeHandles() > 0)
        {
            m_AvailableHeaps.insert(i);
        }
    }
}

std::shared_ptr<GfxDescriptorMemoryPage> GfxDescriptorAllocator::CreateAllocatorPage()
{
    std::shared_ptr<GfxDescriptorMemoryPage> page = std::make_shared<GfxDescriptorMemoryPage>(
        m_Device,
        m_HeapType,
        m_NumDescriptorsPerHeap
    );

    m_HeapPool.emplace_back(page);
    m_AvailableHeaps.insert(m_HeapPool.size() - 1);

    return page;
}
