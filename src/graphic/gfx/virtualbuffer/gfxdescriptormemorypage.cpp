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

#include "gfxdescriptormemorypage.h"

ETH_NAMESPACE_BEGIN

GfxDescriptorMemoryPage::GfxDescriptorMemoryPage(
    wrl::ComPtr<ID3D12Device3> device,
    D3D12_DESCRIPTOR_HEAP_TYPE type,
    uint32_t numDescriptors)
{
    m_DescriptorHeap = std::make_unique<DX12DescriptorHeap>(device, type, numDescriptors);
    m_NumFreeHandles = numDescriptors;
    AddNewBlock(0, m_NumFreeHandles);
}

bool GfxDescriptorMemoryPage::HasSpace(uint32_t numDescriptors) const
{
    return m_FreeListBySize.lower_bound(numDescriptors) != m_FreeListBySize.end();
}

GfxDescriptorAllocation GfxDescriptorMemoryPage::Allocate(uint32_t numDescriptors)
{
    std::lock_guard<std::mutex> lock(m_AllocationMutex);

    if (numDescriptors > m_NumFreeHandles)
        return GfxDescriptorAllocation(); // Default construct a null allocation to signal failure

    auto smallestBlockIterator = m_FreeListBySize.lower_bound(numDescriptors);
    if (smallestBlockIterator == m_FreeListBySize.end())
        return GfxDescriptorAllocation(); // Default construct a null allocation to signal failure

    auto blockSize = smallestBlockIterator->first;
    auto offsetIterator = smallestBlockIterator->second;
    auto offset = offsetIterator->first;

    // Remove the existing free block from the free list
    m_FreeListBySize.erase(smallestBlockIterator);
    m_FreeListByOffset.erase(offsetIterator);
    
    // Compute the new free block that results from splitting this block
    auto newOffset = offset + numDescriptors;
    auto newSize = blockSize - numDescriptors;

    if (newSize > 0)
        AddNewBlock(newOffset, newSize);

    m_NumFreeHandles -= numDescriptors;

    return GfxDescriptorAllocation(
        m_DescriptorHeap->GetHandle(offset),
        numDescriptors,
        m_DescriptorHeap->GetDescriptorHandleStride(),
        this);
}

void GfxDescriptorMemoryPage::Free(GfxDescriptorAllocation&& allocation, uint32_t graphicFrameNumber)
{
    auto offset = m_DescriptorHeap->ComputeOffset(allocation.GetDescriptorHandle());

    std::lock_guard<std::mutex> lock(m_AllocationMutex);

    m_StaleDescriptors.emplace(offset, allocation.GetNumHandles(), graphicFrameNumber);
}

void GfxDescriptorMemoryPage::ReleaseStaleDescriptors(uint64_t graphicFrameNumber)
{
    std::lock_guard<std::mutex> lock(m_AllocationMutex);

    while (!m_StaleDescriptors.empty() &&
        m_StaleDescriptors.front().m_GraphicFrameNumber <= graphicFrameNumber)
    {
        GfxStaleDescriptorInfo& staleDescriptor = m_StaleDescriptors.front();

        auto offset = staleDescriptor.m_Offset;
        auto numDescriptors = staleDescriptor.m_Size;

        FreeBlock(offset, numDescriptors);
        m_StaleDescriptors.pop();
    }
}

void GfxDescriptorMemoryPage::AddNewBlock(uint32_t offset, uint32_t numDescriptors)
{
    auto offsetIterator = m_FreeListByOffset.emplace(offset, numDescriptors);
    auto sizeIterator = m_FreeListBySize.emplace(numDescriptors, offsetIterator.first);

    offsetIterator.first->second.m_BlockLocation = sizeIterator;
}

void GfxDescriptorMemoryPage::FreeBlock(uint32_t offset, uint32_t numDescriptors)
{
    auto nextBlockIterator = m_FreeListByOffset.upper_bound(offset);
    auto prevBlockIterator = nextBlockIterator;

    if (prevBlockIterator!= m_FreeListByOffset.begin())
        --prevBlockIterator;
    else
        prevBlockIterator = m_FreeListByOffset.end(); // no prev block to consider

    m_NumFreeHandles += numDescriptors;

    // Check if the previous block can be merged
    if (prevBlockIterator != m_FreeListByOffset.end() &&
        offset == prevBlockIterator->first + prevBlockIterator->second.m_Size)
    {
        // The previous block is exactly behind the block that is to be freed
        // ///////// |<--- PrevBlock.second (size) --->|<--- size --- >|
        //           |                                 |
        // PrevBlock.first (offset)                  offset
        offset = prevBlockIterator->first;
        numDescriptors += prevBlockIterator->second.m_Size;

        m_FreeListBySize.erase(prevBlockIterator->second.m_BlockLocation);
        m_FreeListByOffset.erase(prevBlockIterator);
    }

    // Check if the next block can be merged
    if (nextBlockIterator != m_FreeListByOffset.end() &&
        offset + numDescriptors == nextBlockIterator->first)
    {
        // The next block is exactly after the block that is to be freed
        // ///////// |<--- size --->|<--- nextBlock.second (size) --- >|
        //           |              |
        //         offset        nextBlock.first (offset)

        numDescriptors += nextBlockIterator->second.m_Size;
        m_FreeListBySize.erase(nextBlockIterator->second.m_BlockLocation);
        m_FreeListByOffset.erase(nextBlockIterator);
    }

    AddNewBlock(offset, numDescriptors);
}

ETH_NAMESPACE_END
