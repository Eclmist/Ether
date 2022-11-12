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

#include "freelistallocator.h"

ETH_NAMESPACE_BEGIN

FreeListAllocator::FreeListAllocator(uint32_t size)
{
    AddBlock(0, size);
}

FreeListAllocation FreeListAllocator::Allocate(uint32_t size)
{
    auto smallestFreeBlockIter = m_SizeToFreeBlocksMap.lower_bound(size);
    uint32_t blockSize = smallestFreeBlockIter->first;
    auto offsetIter = smallestFreeBlockIter->second;
    uint32_t offset = offsetIter->first;

    // Remove the existing free block from the free list.
    m_SizeToFreeBlocksMap.erase(smallestFreeBlockIter);
    m_OffsetToFreeBlockMap.erase(offsetIter);

    // Compute the new free block that results from splitting this block.
    uint32_t newOffset = offset + size;
    uint32_t newSize = blockSize - size;

    // Requested size does not match exactly, split and create new block
    if (newSize > 0)
        AddBlock(newOffset, newSize);

    return { offset, size };
}

bool FreeListAllocator::HasSpace(uint32_t size) const
{
    return m_SizeToFreeBlocksMap.lower_bound(size) != m_SizeToFreeBlocksMap.end();
}

void FreeListAllocator::AddBlock(uint32_t offset, uint32_t size)
{
    auto offsetIter = m_OffsetToFreeBlockMap.emplace(offset, size);
    auto sizeIter = m_SizeToFreeBlocksMap.emplace(size, offsetIter.first);
    offsetIter.first->second.m_SizeToBlockIterator = sizeIter;
}

void FreeListAllocator::FreeBlock(uint32_t offset, uint32_t size)
{
    auto nextBlockIter = m_OffsetToFreeBlockMap.upper_bound(offset);
    auto prevBlockIter = nextBlockIter;

    // If it's not the first block in the list.
    if (prevBlockIter != m_OffsetToFreeBlockMap.begin())
    {
        // Go to the previous block in the list.
        prevBlockIter--;
    }
    else
    {
        // Otherwise, just set it to the end of the list to indicate that no
        // block comes before the one being freed.
        prevBlockIter = m_OffsetToFreeBlockMap.end();
    }
}

ETH_NAMESPACE_END