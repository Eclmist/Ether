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

#include "graphic/hal/dx12includes.h"

class GfxDescriptorMemoryPage;

// This class describes a single allocation in a descriptor page.
// A single allocation may contain multiple, uniformly sized descriptors.
// These descriptors will be in a contiguous block and can be 
// individually accessed if the specific offset is known.
class GfxDescriptorAllocation
{
public:
    GfxDescriptorAllocation();

    GfxDescriptorAllocation(
        D3D12_CPU_DESCRIPTOR_HANDLE descriptor,
        uint32_t numHandles,
        uint32_t descriptorSize,
        GfxDescriptorMemoryPage* page);

    ~GfxDescriptorAllocation();

    // Copy is forbidden
    GfxDescriptorAllocation(const GfxDescriptorAllocation& copy) = delete;
    GfxDescriptorAllocation& operator=(const GfxDescriptorAllocation& copy) = delete;

    // Move is ok
    GfxDescriptorAllocation(GfxDescriptorAllocation&& source);
    GfxDescriptorAllocation& operator=(GfxDescriptorAllocation&& source);

public:
    inline uint32_t GetNumHandles() const { return m_NumHandles; };
    inline bool IsNull() const { return m_Descriptor.ptr == 0; };

public:
    D3D12_CPU_DESCRIPTOR_HANDLE GetDescriptorHandle(uint32_t offset = 0);

private:
    void MoveFrom(GfxDescriptorAllocation& source);
    void Invalidate();
    void Free(uint32_t graphicFrameNumber);

    // The base descriptor
    D3D12_CPU_DESCRIPTOR_HANDLE m_Descriptor;

    uint32_t m_NumHandles;
    uint32_t m_DescriptorSize;

    // A pointer to the page where this allocator came from
    GfxDescriptorMemoryPage* m_Page;
};

