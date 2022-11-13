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

#include "descriptorallocation.h"

ETH_NAMESPACE_BEGIN

DescriptorAllocation::DescriptorAllocation(
    RhiCpuHandle allocBaseCpuHandle,
    RhiGpuHandle allocBaseGpuHandle,
    uint32_t numDescriptors,
    uint32_t descriptorSize,
    uint32_t indexInAllocator,
    DescriptorAllocator* parentAllocator)
    : m_BaseCpuHandle(allocBaseCpuHandle)
    , m_BaseGpuHandle(allocBaseGpuHandle)
    , m_NumDescriptors(numDescriptors)
    , m_DescriptorSize(descriptorSize)
    , m_IndexInAllocator(indexInAllocator)
    , m_Parent(parentAllocator)
    , m_IsValid(true)
{
}

DescriptorAllocation::~DescriptorAllocation() noexcept
{
    if (m_IsValid)
        m_Parent->Free(*this);
}

DescriptorAllocation::DescriptorAllocation(DescriptorAllocation&& move) noexcept
    : m_BaseCpuHandle(move.m_BaseCpuHandle)
    , m_BaseGpuHandle(move.m_BaseGpuHandle)
    , m_NumDescriptors(move.m_NumDescriptors)
    , m_DescriptorSize(move.m_DescriptorSize)
    , m_IndexInAllocator(move.m_IndexInAllocator)
    , m_Parent(move.m_Parent)
    , m_IsValid(move.m_IsValid)
{
    AssertGraphics(move.m_IsValid, "Invalid allocations should never be moved");
    move.m_IsValid = false;
}

DescriptorAllocation& DescriptorAllocation::operator=(DescriptorAllocation&& move) noexcept
{
    AssertGraphics(move.m_IsValid, "Invalid allocations should never be moved");
    m_Parent->Free(*this);

    m_BaseCpuHandle = move.m_BaseCpuHandle;
    m_BaseGpuHandle = move.m_BaseGpuHandle;
    m_NumDescriptors = move.m_NumDescriptors;
    m_DescriptorSize = move.m_DescriptorSize;
    m_IndexInAllocator = move.m_IndexInAllocator;
    m_Parent = move.m_Parent;
    m_IsValid = move.m_IsValid;

    move.m_IsValid = false;

    return *this;
}

uint32_t DescriptorAllocation::GetDescriptorIndex(uint32_t offset) const
{
    return m_IndexInAllocator + offset;
}

RhiCpuHandle DescriptorAllocation::GetCpuHandle(uint32_t offset) const
{
    return { m_BaseCpuHandle.m_Ptr + offset * m_DescriptorSize };
}

RhiGpuHandle DescriptorAllocation::GetGpuHandle(uint32_t offset) const
{
    if (!m_Parent->IsShaderVisible())
        return {};

    return { m_BaseGpuHandle.m_Ptr + offset * m_DescriptorSize };
}

ETH_NAMESPACE_END

