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

#include "linearallocator.h"

Ether::LinearAllocator::LinearAllocator(size_t capacity)
    : MemoryAllocator(capacity)
    , m_Offset(0)
{
}

std::unique_ptr<Ether::MemoryAllocation> Ether::LinearAllocator::Allocate(SizeAlign sizeAlign)
{
    if (!HasSpace(sizeAlign))
        return nullptr;

    size_t alignedOffset = AlignUp(m_Offset, sizeAlign.m_Alignment);
    size_t alignedSize = AlignUp(sizeAlign.m_Size, sizeAlign.m_Alignment);

    m_Offset = alignedOffset + alignedSize;
    return std::make_unique<Ether::LinearAllocation>(alignedOffset, alignedSize);
}

void Ether::LinearAllocator::Free(std::unique_ptr<MemoryAllocation>&& alloc)
{
    // Do nothing
    throw std::runtime_error("Free is not supported by linear allocator. Reset the allocator once full.");
}

bool Ether::LinearAllocator::HasSpace(SizeAlign sizeAlign) const
{
    size_t alignedSize = AlignUp(sizeAlign.m_Size, sizeAlign.m_Alignment);
    size_t alignedOffset = AlignUp(m_Offset, sizeAlign.m_Alignment);
    return alignedOffset + alignedSize <= m_Capacity;
}

void Ether::LinearAllocator::Reset()
{
    m_Offset = 0;
}
