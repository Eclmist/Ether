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

#include "common/memory/freelistallocator.h"
#include "graphics/rhi/rhidescriptorheap.h"
#include "graphics/memory/descriptorallocation.h"
#include <queue>

namespace Ether::Graphics
{
    class DescriptorAllocator : public FreeListAllocator
    {
    public:
        DescriptorAllocator(
            RhiDescriptorHeapType type,
            bool isShaderVisible = false,
            uint32_t maxHeapSize = 1 << 12 // 4096
        );

        ~DescriptorAllocator() = default;

    public:
        std::unique_ptr<MemoryAllocation> Allocate(SizeAlign = {1, 1}) override;
        void Free(std::unique_ptr<MemoryAllocation>&& alloc) override;

    public:
        inline RhiDescriptorHeap& GetDescriptorHeap() const { return *m_DescriptorHeap; }
        inline uint32_t GetMaxNumDescriptors() const { return m_MaxDescriptors; }
        inline bool IsShaderVisible() const { return m_IsShaderVisible; }

    private:
        friend class DescriptorAllocation;
        void Free(const DescriptorAllocation& allocation);
        void ReclaimStaleAllocations(uint32_t numIndices);

    private:
        uint32_t m_MaxDescriptors;
        bool m_IsShaderVisible;

        RhiDescriptorHeapType m_HeapType;
        std::unique_ptr<RhiDescriptorHeap> m_DescriptorHeap;
        std::queue<FreeListAllocation> m_StaleAllocations;
    };
}
