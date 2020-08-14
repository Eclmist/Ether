/*
    This file is part of Ether, an open-source DirectX 12 renderer.

    Copyright (c) 2020-2021 Samuel Van Allen - All rights reserved.

    Ether is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "gfxdescriptorallocation.h"
#include "graphic/virtualbuffers/gfxdescriptormemorypage.h"

GfxDescriptorAllocation::GfxDescriptorAllocation()
    : m_Descriptor{0}
    , m_NumHandles(0)
    , m_DescriptorSize(0)
    , m_Page(nullptr)
{
}

GfxDescriptorAllocation::GfxDescriptorAllocation(
    D3D12_CPU_DESCRIPTOR_HANDLE descriptor,
    uint32_t numHandles,
    uint32_t descriptorSize,
    GfxDescriptorMemoryPage* page)
    : m_Descriptor(descriptor)
    , m_NumHandles(numHandles)
    , m_DescriptorSize(descriptorSize)
    , m_Page(page)
{
    assert(page != nullptr && "Attempting to create a descriptor allocation with an invalid page!");
}

GfxDescriptorAllocation::~GfxDescriptorAllocation()
{
    // Todo Critical: Create GfxContext, which would store the graphic frame number to pass into here
    Free(0);
}

GfxDescriptorAllocation::GfxDescriptorAllocation(GfxDescriptorAllocation&& source)
{
    MoveFrom(source);
}

GfxDescriptorAllocation& GfxDescriptorAllocation::operator=(GfxDescriptorAllocation&& source)
{
    // Todo Critical: Create GfxContext, which would store the graphic frame number to pass into here
    Free(0);
    MoveFrom(source);
    return *this;
}

D3D12_CPU_DESCRIPTOR_HANDLE GfxDescriptorAllocation::GetDescriptorHandle(uint32_t offset)
{
    assert(offset < m_NumHandles);
    return { m_Descriptor.ptr + (m_DescriptorSize * offset) };
}

void GfxDescriptorAllocation::MoveFrom(GfxDescriptorAllocation& source)
{
    m_Descriptor = source.m_Descriptor;
    m_NumHandles = source.m_NumHandles;
    m_DescriptorSize = source.m_DescriptorSize;
    m_Page = source.m_Page;

    source.Invalidate();
}

void GfxDescriptorAllocation::Invalidate()
{
    m_Descriptor.ptr = 0;
    m_NumHandles = 0;
    m_DescriptorSize = 0;
    m_Page = nullptr;
}

void GfxDescriptorAllocation::Free(uint32_t graphicFrameNumber)
{
    if (!IsNull() && m_Page)
    {
        m_Page->Free(std::move(*this), graphicFrameNumber);
        Invalidate();
    }
}

