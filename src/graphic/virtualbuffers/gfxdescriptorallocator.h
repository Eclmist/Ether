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

#include "system/system.h"

#include "graphic/hal/dx12includes.h"
#include "graphic/virtualbuffers/gfxdescriptorallocation.h"
#include "graphic/virtualbuffers/gfxdescriptormemorypage.h"

class GfxDescriptorAllocator
{
public:
    GfxDescriptorAllocator(
        wrl::ComPtr<ID3D12Device3> device,
        D3D12_DESCRIPTOR_HEAP_TYPE type,
        uint32_t numDescriptorsPerHeap = 256);

    ~GfxDescriptorAllocator();

public:
    GfxDescriptorAllocation Allocate(uint32_t numDescriptors = 1);
    void ReleaseStaleDescriptors(uint64_t frameNumber);

private:
    std::shared_ptr<GfxDescriptorMemoryPage> CreateAllocatorPage();

    wrl::ComPtr<ID3D12Device3> m_Device;

    D3D12_DESCRIPTOR_HEAP_TYPE m_HeapType;
    uint32_t m_NumDescriptorsPerHeap;

    std::vector<std::shared_ptr<GfxDescriptorMemoryPage>> m_HeapPool;
    std::set<size_t> m_AvailableHeaps;

    std::mutex m_AllocationMutex;
};
