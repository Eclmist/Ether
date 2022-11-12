/*
    This file is part of Ether, an open-source DirectX 12 renderer.

    Copyright (c) 2020-2022 Samuel Huang - All rights reserved.

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

#include "descriptorallocator.h"
#include "graphic/rhi/rhidevice.h"

ETH_NAMESPACE_BEGIN

DescriptorAllocator::DescriptorAllocator(
    RhiDescriptorHeapType type,
    bool isShaderVisible,
    uint32_t maxHeapSize)
    : m_HeapType(type)
    , m_MaxDescriptors(maxHeapSize)
    , m_IsShaderVisible(isShaderVisible)
    , m_FreeListAllocator(maxHeapSize)
{
    GraphicCore::GetDevice()->CreateDescriptorHeap({
        type,
        isShaderVisible ? RhiDescriptorHeapFlag::ShaderVisible : RhiDescriptorHeapFlag::None,
        maxHeapSize
    }, m_DescriptorHeap);
}

std::shared_ptr<DescriptorAllocation> DescriptorAllocator::Allocate(uint32_t numDescriptors)
{
    if (!m_FreeListAllocator.HasSpace(numDescriptors))
    {
        bool result = ReclaimStaleAllocations(numDescriptors);
        if (!result)
        {
            LogGraphicsError("Descriptor allocation failed - heap is already full");
            throw std::bad_alloc();
        }
    }

    auto alloc = m_FreeListAllocator.Allocate(numDescriptors);
    uint32_t descriptorIdx = alloc.m_Offset;
    uint32_t descriptorSize = m_DescriptorHeap->GetHandleIncrementSize();
    RhiCpuHandle allocBaseCpuHandle = { m_DescriptorHeap->GetBaseCpuHandle().m_Ptr + descriptorIdx * descriptorSize };
    RhiGpuHandle allocBaseGpuHandle = m_IsShaderVisible
        ? RhiGpuHandle{ m_DescriptorHeap->GetBaseGpuHandle().m_Ptr + descriptorIdx * descriptorSize }
        : RhiGpuHandle{};

    return std::make_shared<DescriptorAllocation>(
        allocBaseCpuHandle,
        allocBaseGpuHandle,
        numDescriptors,
        descriptorSize,
        descriptorIdx,
        this);
}

void DescriptorAllocator::Free(const DescriptorAllocation& allocation)
{
    // Mark index as free, but don't actually do anything because
    // it may still be in flight. We will reclaim these when we completely 
    // run out of space

    m_StaleAllocations.emplace(allocation.GetDescriptorIndex(), allocation.GetNumDescriptors());
}

bool DescriptorAllocator::ReclaimStaleAllocations(uint32_t numIndices)
{

    while (numIndices > 0)
    {
        if (m_StaleAllocations.empty())
        {
            LogGraphicsWarning("Descriptor heap is full - no stale indices to reclaim");
            return false;
        }

        StaleAllocation staleAlloc = m_StaleAllocations.front();
        m_StaleAllocations.pop();
        m_FreeListAllocator.FreeBlock(staleAlloc.m_IndexInHeap, staleAlloc.m_NumDescriptors);

        numIndices -= staleAlloc.m_NumDescriptors;
    }

    return true;
}

ETH_NAMESPACE_END

