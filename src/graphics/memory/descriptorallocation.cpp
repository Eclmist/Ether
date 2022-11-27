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

#include "graphics/memory/descriptorallocation.h"
#include "graphics/memory/descriptorallocator.h"

Ether::Graphics::DescriptorAllocation::DescriptorAllocation(
    RhiCpuAddress allocBaseCpuAddr,
    RhiGpuAddress allocBaseGpuAddr,
    size_t numDescriptors,
    size_t descriptorSize,
    size_t indexInAllocator,
    DescriptorAllocator* parentAllocator)
    : FreeListAllocation(indexInAllocator, numDescriptors)
    , m_BaseCpuAddress(allocBaseCpuAddr)
    , m_BaseGpuAddress(allocBaseGpuAddr)
    , m_DescriptorSize(descriptorSize)
    , m_Parent(parentAllocator)
    , m_IsValid(true)
{
}

Ether::Graphics::DescriptorAllocation::~DescriptorAllocation() noexcept
{
    if (m_IsValid)
        m_Parent->Free(*this);
}

Ether::Graphics::DescriptorAllocation::DescriptorAllocation(DescriptorAllocation&& move) noexcept
    :  FreeListAllocation(move.m_Offset, move.m_Size)
    , m_BaseCpuAddress(move.m_BaseCpuAddress)
    , m_BaseGpuAddress(move.m_BaseGpuAddress)
    , m_DescriptorSize(move.m_DescriptorSize)
    , m_Parent(move.m_Parent)
    , m_IsValid(move.m_IsValid)
{
    move.m_IsValid = false;
}

Ether::Graphics::DescriptorAllocation& Ether::Graphics::DescriptorAllocation::operator=(DescriptorAllocation&& move) noexcept
{
    m_Parent->Free(*this);

    m_BaseCpuAddress = move.m_BaseCpuAddress;
    m_BaseGpuAddress = move.m_BaseGpuAddress;
    m_Size = move.m_Size;
    m_DescriptorSize = move.m_DescriptorSize;
    m_Offset = move.m_Offset;
    m_Parent = move.m_Parent;
    m_IsValid = move.m_IsValid;

    move.m_IsValid = false;

    return *this;
}

size_t Ether::Graphics::DescriptorAllocation::GetDescriptorIndex(size_t localIndex) const
{
    return m_Offset + localIndex;
}

Ether::Graphics::RhiCpuAddress Ether::Graphics::DescriptorAllocation::GetCpuAddress(size_t localIndex) const
{
    return m_BaseCpuAddress + localIndex * m_DescriptorSize;
}

Ether::Graphics::RhiGpuAddress Ether::Graphics::DescriptorAllocation::GetGpuAddress(size_t localIndex) const
{
    if (!m_Parent->IsShaderVisible())
        return {};

    return m_BaseGpuAddress + localIndex * m_DescriptorSize;
}

