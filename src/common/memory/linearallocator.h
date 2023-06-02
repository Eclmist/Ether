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

#include "common/memory/memoryallocator.h"

namespace Ether
{
class ETH_COMMON_DLL LinearAllocation : public MemoryAllocation
{
public:
    LinearAllocation() = default;
    LinearAllocation(size_t offset, size_t size)
        : m_Offset(offset)
        , m_Size(size)
    {
    }
    ~LinearAllocation() override {}

    virtual size_t GetOffset() const override { return m_Offset; }
    virtual size_t GetSize() const override { return m_Size; }
    virtual void* GetBaseCpuHandle() const override { return nullptr; }

protected:
    size_t m_Offset;
    size_t m_Size;
};

class ETH_COMMON_DLL LinearAllocator : public MemoryAllocator
{
public:
    LinearAllocator(size_t capacity);
    ~LinearAllocator() = default;

public:
    std::unique_ptr<MemoryAllocation> Allocate(SizeAlign sizeAlign) override;
    void Free(std::unique_ptr<MemoryAllocation>&& alloc) override;
    bool HasSpace(SizeAlign sizeAlign) const override;
    void Reset() override;

protected:
    size_t m_Offset;
};
} // namespace Ether
