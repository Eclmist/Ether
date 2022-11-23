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
#include "graphics/memory/uploadbufferallocatorpage.h"

namespace Ether::Graphics
{
    class UploadBufferAllocator : public MemoryAllocator
    {
    public:
        UploadBufferAllocator(size_t pageSize = 1 << 26 /*(64MiB)*/);
        ~UploadBufferAllocator() = default;

    public:
        std::unique_ptr<MemoryAllocation> Allocate(SizeAlign sizeAlign) override;
        void Free(std::unique_ptr<MemoryAllocation>&& alloc) override;
        bool HasSpace(SizeAlign sizeAlign) const override;
        void Reset() override;

    private:
        UploadBufferAllocatorPage* GetNextAvailablePage();
        void AllocatePage();

    private:
        std::vector<std::unique_ptr<UploadBufferAllocatorPage>> m_PagePool;
        std::vector<UploadBufferAllocatorPage*> m_AvaliablePages;

        UploadBufferAllocatorPage* m_CurrentPage;
        const size_t m_PageSize;
    };
}