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

    if (prevBlockIter != m_OffsetToFreeBlockMap.begin())
        prevBlockIter--;
    else
        prevBlockIter = m_OffsetToFreeBlockMap.end();

    // The previous block is exactly behind the block that is to be freed.
    if (prevBlockIter != m_OffsetToFreeBlockMap.end() && offset == prevBlockIter->first + prevBlockIter->second.m_Size)
    {
        MergeBlock(prevBlockIter, { offset, size });
        return;
    }

    // The next block is exactly in front of the block that is to be freed.
    if (nextBlockIter != m_OffsetToFreeBlockMap.end() && offset + size == nextBlockIter->first)
    {
        MergeBlock(nextBlockIter, { offset, size });
        return;
    }

    // The freed block is not next to any other freed block
    AddBlock(offset, size);
}

void FreeListAllocator::MergeBlock(OffsetToBlockMap::iterator blockIter, FreeListAllocation newBlock)
{
    AssertEngine(
        newBlock.m_Offset + newBlock.m_Size == blockIter->first ||
        blockIter->first + blockIter->second.m_Size == newBlock.m_Offset,
        "FreeListAllocator - Misaligned blocks cannot be merged");

    uint32_t newSize = newBlock.m_Size + blockIter->second.m_Size;
    uint32_t newOffset = (newBlock.m_Offset < blockIter->first) ? newBlock.m_Offset : blockIter->first;

    m_SizeToFreeBlocksMap.erase(blockIter->second.m_SizeToBlockIterator);
    m_OffsetToFreeBlockMap.erase(blockIter);
    AddBlock(newOffset, newSize);
}

ETH_NAMESPACE_END