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

#include "gfxresourcememorypage.h"
#include "graphic/gfx/virtualbuffer/gfxmemoryutils.h"

GfxResourceMemoryPage::GfxResourceMemoryPage(
    wrl::ComPtr<ID3D12Device3> device,
    size_t sizeInBytes)
    : m_PageSize(sizeInBytes)
    , m_Offset(0)
    , m_CPUMemoryPtr(nullptr)
    , m_GPUMemoryPtr(D3D12_GPU_VIRTUAL_ADDRESS(0))
{
    m_Resource = std::make_unique<DX12CommittedResource>(
        device,
        sizeInBytes,
        D3D12_HEAP_TYPE_UPLOAD,
        D3D12_HEAP_FLAG_NONE,
        D3D12_RESOURCE_STATE_GENERIC_READ
    );

    m_GPUMemoryPtr = m_Resource->Get()->GetGPUVirtualAddress();
    m_Resource->Get()->Map(0, nullptr, &m_CPUMemoryPtr);
}

GfxResourceMemoryPage::~GfxResourceMemoryPage()
{
    m_Resource->Get()->Unmap(0, nullptr);
    m_CPUMemoryPtr = nullptr;
    m_GPUMemoryPtr = D3D12_GPU_VIRTUAL_ADDRESS(0);
}

bool GfxResourceMemoryPage::HasSpace(size_t sizeInBytes, size_t alignment) const
{
    size_t alignedSize = AlignUp(sizeInBytes, alignment);
    size_t alignedOffset = AlignUp(m_Offset, alignment);

    return alignedOffset + alignedSize <= m_PageSize;
}

GfxResourceAllocation GfxResourceMemoryPage::Allocate(size_t sizeInBytes, size_t alignment)
{
    if (!HasSpace(sizeInBytes, alignment))
    {
        throw std::bad_alloc();
    }

    size_t alignedSize = AlignUp(sizeInBytes, alignment);
    m_Offset = AlignUp(m_Offset, alignment);

    GfxResourceAllocation allocation;
    allocation.m_CPUMemoryPtr = static_cast<uint8_t*>(m_CPUMemoryPtr) + m_Offset;
    allocation.m_GPUMemoryPtr = m_GPUMemoryPtr + m_Offset;

    m_Offset += alignedSize;

    return allocation;
}

void GfxResourceMemoryPage::Reset()
{
    m_Offset = 0;
}

