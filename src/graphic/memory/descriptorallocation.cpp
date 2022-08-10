/*
    This file is part of Ether, an open-source DirectX 12 renderer.

    Copyright (c) 2020-2022 Samuel Huang - All rights reserved.

    Ether is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "descriptorallocation.h"
#include "descriptorallocatorpage.h"

ETH_NAMESPACE_BEGIN

DescriptorAllocation::DescriptorAllocation(
    RhiCpuHandle allocBaseHandle,
    uint32_t numDescriptors,
    uint32_t pageOffset,
    uint32_t descriptorSize,
    DescriptorAllocatorPage* parentPage)
    : m_AllocationBaseHandle(allocBaseHandle)
    , m_NumDescriptors(numDescriptors)
    , m_PageOffset(pageOffset)
    , m_DescriptorSize(descriptorSize)
    , m_ParentPage(parentPage)
{
}

DescriptorAllocation::~DescriptorAllocation()
{
    Free();
}

RhiCpuHandle DescriptorAllocation::GetDescriptorHandle(uint32_t offset) const
{
    AssertGraphics(offset < m_NumDescriptors, "Specified offset is greater than total number of descriptor handles");
    return { m_AllocationBaseHandle.m_Ptr + (m_DescriptorSize * offset) };
}

void DescriptorAllocation::Free()
{
    AssertGraphics(m_ParentPage != nullptr, "A descriptor allocation has an invalid parent page");

    m_ParentPage->Free(std::move(*this), GraphicCore::GetFrameNumber());
    m_AllocationBaseHandle.m_Ptr = 0;
    m_NumDescriptors = 0;
    m_PageOffset = 0;
    m_DescriptorSize = 0;
    m_ParentPage = nullptr;
}

ETH_NAMESPACE_END

