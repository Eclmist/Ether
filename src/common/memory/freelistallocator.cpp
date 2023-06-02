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

#include "common/memory/freelistallocator.h"

Ether::FreeListAllocator::FreeListAllocator(size_t capacity)
    : MemoryAllocator(capacity)
{
    AddBlock(0, capacity);
}

std::unique_ptr<Ether::MemoryAllocation> Ether::FreeListAllocator::Allocate(SizeAlign sizeAlign)
{
    auto smallestFreeBlockIter = FindFreeBlock(sizeAlign);
    if (smallestFreeBlockIter == nullptr)
        return nullptr;

    uint32_t blockSize = smallestFreeBlockIter->m_Size;
    uint32_t offset = smallestFreeBlockIter->m_SizeToOffsetIter->second->first;
    uint32_t alignedOffset = AlignUp(offset, sizeAlign.m_Alignment);
    uint32_t alignedSize = AlignUp(sizeAlign.m_Size, sizeAlign.m_Alignment);

    // Remove the existing free block from the free list.
    auto offsetToBlockIter = smallestFreeBlockIter->m_SizeToOffsetIter->second;
    auto m_SizeToOffsetIter = smallestFreeBlockIter->m_SizeToOffsetIter;
    m_OffsetToBlockMap.erase(offsetToBlockIter);
    m_SizeToOffsetMap.erase(m_SizeToOffsetIter);

    // Compute the new left free block (due to alignment requirements) from splitting this block
    if (alignedOffset - offset > 0)
        AddBlock(offset, alignedOffset - offset);

    // Compute the new right free block that results from splitting this block.
    uint32_t remainingSize = blockSize - alignedSize - (alignedOffset - offset);
    if (remainingSize > 0)
        AddBlock(alignedOffset + alignedSize, remainingSize);

    return std::make_unique<FreeListAllocation>(alignedOffset, alignedSize);
}

void Ether::FreeListAllocator::Free(std::unique_ptr<MemoryAllocation>&& alloc)
{
    FreeBlock(alloc->GetOffset(), alloc->GetSize());
}

bool Ether::FreeListAllocator::HasSpace(SizeAlign sizeAlign) const
{
    return FindFreeBlock(sizeAlign) != nullptr;
}

void Ether::FreeListAllocator::Reset()
{
    m_OffsetToBlockMap.clear();
    m_SizeToOffsetMap.clear();
    AddBlock(0, m_Capacity);
}

Ether::FreeListAllocator::FreeBlockInfo* Ether::FreeListAllocator::FindFreeBlock(SizeAlign sizeAlign) const
{
    size_t minimumBlockSize = AlignUp(sizeAlign.m_Size, sizeAlign.m_Alignment);

    for (auto iter = m_SizeToOffsetMap.lower_bound(minimumBlockSize); iter != m_SizeToOffsetMap.end(); ++iter)
    {
        size_t blockSize = iter->first;
        size_t blockOffset = iter->second->first;
        size_t alignedOffset = AlignUp(blockOffset, sizeAlign.m_Alignment);
        size_t alignedSize = AlignUp(blockSize, sizeAlign.m_Alignment);
        size_t leftPadding = alignedOffset - blockOffset;

        if (blockSize - leftPadding >= alignedSize)
            return &iter->second->second;
    }

    return nullptr;
}

void Ether::FreeListAllocator::AddBlock(size_t offset, size_t size)
{
    auto offsetIter = m_OffsetToBlockMap.emplace(offset, size);
    auto sizeIter = m_SizeToOffsetMap.emplace(size, offsetIter.first);
    offsetIter.first->second.m_SizeToOffsetIter = sizeIter;
}

void Ether::FreeListAllocator::FreeBlock(size_t offset, size_t size)
{
    auto nextBlockIter = m_OffsetToBlockMap.upper_bound(offset);
    auto prevBlockIter = nextBlockIter;

    if (prevBlockIter != m_OffsetToBlockMap.begin())
        prevBlockIter--;
    else
        prevBlockIter = m_OffsetToBlockMap.end();

    // The previous block is exactly behind the block that is to be freed.
    if (prevBlockIter != m_OffsetToBlockMap.end() && offset == prevBlockIter->first + prevBlockIter->second.m_Size)
    {
        MergeBlock(prevBlockIter, { offset, size });
        return;
    }

    // The next block is exactly in front of the block that is to be freed.
    if (nextBlockIter != m_OffsetToBlockMap.end() && offset + size == nextBlockIter->first)
    {
        MergeBlock(nextBlockIter, { offset, size });
        return;
    }

    // The freed block is not next to any other freed block
    AddBlock(offset, size);
}

void Ether::FreeListAllocator::MergeBlock(OffsetToBlockMap::iterator blockIter, FreeListAllocation newBlock)
{
    assert(
        (newBlock.GetOffset() + newBlock.GetSize() == blockIter->first ||
         blockIter->first + blockIter->second.m_Size == newBlock.GetOffset()) &&
        "FreeListAllocator - Misaligned blocks cannot be merged");

    size_t newSize = newBlock.GetSize() + blockIter->second.m_Size;
    size_t newOffset = (newBlock.GetOffset() < blockIter->first) ? newBlock.GetOffset() : blockIter->first;

    m_SizeToOffsetMap.erase(blockIter->second.m_SizeToOffsetIter);
    m_OffsetToBlockMap.erase(blockIter);
    AddBlock(newOffset, newSize);
}
