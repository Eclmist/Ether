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

#pragma once

#include "gfxdescriptormemorypage.h"

GfxDescriptorMemoryPage::GfxDescriptorMemoryPage(
    wrl::ComPtr<ID3D12Device3> device,
    D3D12_DESCRIPTOR_HEAP_TYPE type,
    uint32_t numDescriptors)
{
    m_DescriptorHeap = std::make_unique<DX12DescriptorHeap>(device, type, numDescriptors);
}

bool GfxDescriptorMemoryPage::HasSpace(uint32_t numDescriptors)
{
    return false;

}

GfxDescriptorAllocation GfxDescriptorMemoryPage::Allocate(uint32_t numDescriptors)
{
    GfxDescriptorAllocation allocation;
    allocation.m_CPUMemoryPtr = nullptr;
    allocation.m_GPUMemoryPtr = 1;

    return allocation;
}

void GfxDescriptorMemoryPage::Free(GfxDescriptorAllocation&& descriptionHandle, uint64_t frameNumber)
{

}

void GfxDescriptorMemoryPage::ReleaseStaleDescriptors(uint64_t frameNumber)
{

}

uint32_t GfxDescriptorMemoryPage::ComputeOffset(D3D12_CPU_DESCRIPTOR_HANDLE handle)
{
    return 0;
}

void GfxDescriptorMemoryPage::AddNewBlock(uint32_t offset, uint32_t numDescriptors)
{

}

void GfxDescriptorMemoryPage::FreeBlock(uint32_t offset, uint32_t numDescriptors)
{

}
