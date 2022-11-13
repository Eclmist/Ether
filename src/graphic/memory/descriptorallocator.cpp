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
    , m_StaleAllocations()
{
    GraphicCore::GetDevice()->CreateDescriptorHeap({
        type,
        isShaderVisible ? RhiDescriptorHeapFlag::ShaderVisible : RhiDescriptorHeapFlag::None,
        maxHeapSize
    }, m_DescriptorHeap);
}

DescriptorAllocation DescriptorAllocator::Allocate(uint32_t numDescriptors)
{
    if (!m_FreeListAllocator.HasSpace(numDescriptors))
        ReclaimStaleAllocations(numDescriptors);

    FreeListAllocation alloc = m_FreeListAllocator.Allocate(numDescriptors);
    uint32_t descriptorIdx = alloc.m_Offset;
    uint32_t descriptorSize = m_DescriptorHeap->GetHandleIncrementSize();
    RhiCpuHandle allocBaseCpuHandle = { m_DescriptorHeap->GetBaseCpuHandle().m_Ptr + descriptorIdx * descriptorSize };
    RhiGpuHandle allocBaseGpuHandle = m_IsShaderVisible
        ? RhiGpuHandle{ m_DescriptorHeap->GetBaseGpuHandle().m_Ptr + descriptorIdx * descriptorSize }
        : RhiGpuHandle{};

    return DescriptorAllocation(
        allocBaseCpuHandle,
        allocBaseGpuHandle,
        numDescriptors,
        descriptorSize,
        descriptorIdx,
        this
    );
}

void DescriptorAllocator::Free(const DescriptorAllocation& allocation)
{
    m_StaleAllocations.emplace(allocation.GetDescriptorIndex(), allocation.GetNumDescriptors());
}

void DescriptorAllocator::ReclaimStaleAllocations(uint32_t numIndices)
{
    while (numIndices > 0)
    {
        if (m_StaleAllocations.empty())
        {
            LogGraphicsError("Descriptor heap is full - no stale indices to reclaim");
            LogGraphicsError("Descriptor allocation failed - heap is already full");
            throw std::bad_alloc();
        }

        StaleAllocation staleAlloc = m_StaleAllocations.front();
        m_StaleAllocations.pop();
        m_FreeListAllocator.FreeBlock(staleAlloc.m_IndexInHeap, staleAlloc.m_NumDescriptors);

        numIndices -= staleAlloc.m_NumDescriptors;
    }
}

ETH_NAMESPACE_END

