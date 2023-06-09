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

#include "graphics/pch.h"
#include "common/memory/freelistallocator.h"

namespace Ether::Graphics
{
class DescriptorAllocator;

class DescriptorAllocation : public FreeListAllocation
{
public:
    DescriptorAllocation(
        RhiCpuAddress allocBaseCpuHandle,
        RhiGpuAddress allocBaseGpuHandle,
        size_t numDescriptors,
        size_t descriptorSize,
        size_t indexInAllocator,
        DescriptorAllocator* parentAllocator);

    ~DescriptorAllocation() noexcept;

    DescriptorAllocation(const DescriptorAllocation& copy) = delete;
    DescriptorAllocation& operator=(const DescriptorAllocation& copy) = delete;
    DescriptorAllocation(DescriptorAllocation&& move) noexcept;
    DescriptorAllocation& operator=(DescriptorAllocation&& move) noexcept;

public:
    size_t GetOffset() const override { return m_Offset; }
    size_t GetSize() const override { return m_Size; }

public:
    inline size_t GetNumDescriptors() const { return m_Size; }

public:
    size_t GetDescriptorIndex(size_t localIndex = 0) const;
    RhiCpuAddress GetCpuAddress(size_t localIndex = 0) const;
    RhiGpuAddress GetGpuAddress(size_t localIndex = 0) const;

private:
    RhiCpuAddress m_BaseCpuAddress;
    RhiGpuAddress m_BaseGpuAddress;

    size_t m_DescriptorSize;

    DescriptorAllocator* m_Parent;
    bool m_IsValid;
};
} // namespace Ether::Graphics
