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

#include "graphics/core.h"
#include "graphics/memory/uploadbufferallocatorpage.h"

Ether::Graphics::UploadBufferAllocatorPage::UploadBufferAllocatorPage(size_t capacity)
    : LinearAllocator(capacity)
{
    RhiCommitedResourceDesc desc = {};
    desc.m_HeapType = RhiHeapType::Upload;
    desc.m_State = RhiResourceState::GenericRead;
    desc.m_ResourceDesc = RhiCreateBufferResourceDesc(capacity);
    desc.m_Name = "UploadBufferAllocator::UploadHeap";

    m_UploadHeap = Core::GetDevice().CreateCommittedResource(desc);
    m_UploadHeap->Map(&m_BaseAddress);
}

Ether::Graphics::UploadBufferAllocatorPage::~UploadBufferAllocatorPage()
{
    m_UploadHeap->Unmap();
}

std::unique_ptr<Ether::MemoryAllocation> Ether::Graphics::UploadBufferAllocatorPage::Allocate(SizeAlign sizeAlign)
{
    std::unique_ptr<MemoryAllocation> baseAlloc = LinearAllocator::Allocate(sizeAlign);

    if (baseAlloc == nullptr)
        return nullptr;

    return std::make_unique<UploadBufferAllocation>(baseAlloc->GetOffset(), baseAlloc->GetSize(), m_BaseAddress, m_UploadHeap->GetGpuAddress());
}

