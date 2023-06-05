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

#include "graphics/graphiccore.h"
#include "graphics/memory/descriptorallocator.h"
#include "graphics/rhi/rhidevice.h"

Ether::Graphics::DescriptorAllocator::DescriptorAllocator(
    RhiDescriptorHeapType type,
    bool isShaderVisible,
    size_t maxHeapSize)
    : FreeListAllocator(maxHeapSize)
    , m_MaxDescriptors(maxHeapSize)
    , m_IsShaderVisible(isShaderVisible)
{
    m_DescriptorHeap = GraphicCore::GetDevice().CreateDescriptorHeap(
        { type, isShaderVisible ? RhiDescriptorHeapFlag::ShaderVisible : RhiDescriptorHeapFlag::None, maxHeapSize });
}

std::unique_ptr<Ether::MemoryAllocation> Ether::Graphics::DescriptorAllocator::Allocate(SizeAlign sizeAlign)
{
    std::unique_ptr<MemoryAllocation> baseAlloc = FreeListAllocator::Allocate(sizeAlign);

    if (baseAlloc == nullptr)
    {
        ReclaimStaleAllocations(sizeAlign.m_Size);
        baseAlloc = FreeListAllocator::Allocate(sizeAlign);
    }

    size_t descriptorIdx = baseAlloc->GetOffset();
    size_t descriptorSize = m_DescriptorHeap->GetHandleIncrementSize();

    RhiCpuAddress allocBaseCpuHandle = m_DescriptorHeap->GetBaseCpuAddress() + descriptorIdx * descriptorSize;
    RhiGpuAddress allocBaseGpuHandle = m_IsShaderVisible
                                           ? m_DescriptorHeap->GetBaseGpuAddress() + descriptorIdx * descriptorSize
                                           : NullAddress;

    return std::make_unique<DescriptorAllocation>(
        allocBaseCpuHandle,
        allocBaseGpuHandle,
        sizeAlign.m_Size,
        descriptorSize,
        baseAlloc->GetOffset(),
        this);
}

void Ether::Graphics::DescriptorAllocator::Free(std::unique_ptr<Ether::MemoryAllocation>&& alloc)
{
    alloc.reset();
}

void Ether::Graphics::DescriptorAllocator::Free(const DescriptorAllocation& allocation)
{
    m_StaleAllocations.emplace(allocation.GetDescriptorIndex(), allocation.GetNumDescriptors());
}

void Ether::Graphics::DescriptorAllocator::ReclaimStaleAllocations(size_t numIndices)
{
    while (!m_StaleAllocations.empty())
    {
        FreeListAllocation staleAlloc = m_StaleAllocations.front();
        m_StaleAllocations.pop();
        FreeBlock(staleAlloc.GetOffset(), staleAlloc.GetSize());

        if (numIndices > 0)
            numIndices -= staleAlloc.GetSize();
    }

    if (numIndices > 0)
    {
        LogGraphicsFatal("Descriptor heap is full - no stale indices to reclaim");
        LogGraphicsFatal("Descriptor allocation failed - heap is already full");
        throw std::bad_alloc();
    }
}
