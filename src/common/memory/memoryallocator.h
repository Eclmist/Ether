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

#pragma once

#include "common/common.h"

namespace Ether
{
    struct ETH_COMMON_DLL SizeAlign
    {
        SizeAlign(size_t size, size_t align = 1)
            : m_Size(size)
            , m_Alignment(align) {}

        size_t m_Size;
        size_t m_Alignment;
    };

    class ETH_COMMON_DLL MemoryAllocation
    {
    public:
        MemoryAllocation() = default;
        virtual ~MemoryAllocation() = default;

    public:
        virtual size_t GetOffset() const = 0;
        virtual size_t GetSize() const = 0;
        virtual void* GetBaseAddress() const = 0;
        virtual void* GetAddress() const { return (uint8_t*)GetBaseAddress() + GetOffset(); }
    };

    class ETH_COMMON_DLL MemoryAllocator : public NonCopyable, public NonMovable
    {
    public:
        MemoryAllocator(size_t capacity) : m_Capacity(capacity) {}
        virtual ~MemoryAllocator() = default;

    public:
        virtual std::unique_ptr<MemoryAllocation> Allocate(SizeAlign sizeAlign) = 0;
        virtual void Free(std::unique_ptr<MemoryAllocation>&& alloc) = 0;
        virtual bool HasSpace(SizeAlign sizeAlign) const = 0;
        virtual void Reset() = 0;

    protected:
        size_t m_Capacity;
    };
}
