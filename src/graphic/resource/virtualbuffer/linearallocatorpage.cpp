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

#include "linearallocatorpage.h"
#include "memoryutils.h"

ETH_NAMESPACE_BEGIN

LinearAllocatorPage::LinearAllocatorPage(size_t size)
    : m_Size(size)
    , m_Offset(0)
    , m_CpuAddress(nullptr)
{
    ASSERT_SUCCESS(g_GraphicDevice->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(size),
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&m_Resource)
    ));

    m_GpuAddress = m_Resource->GetGPUVirtualAddress();
    m_Resource->Map(0, nullptr, &m_CpuAddress);
    m_Resource->SetName(L"LinearAllocator::LinearAllocatorPage");
}

bool LinearAllocatorPage::HasSpace(size_t size, size_t alignment)
{
    size_t alignedSize = AlignUp(size, alignment);
    size_t alignedOffset = AlignUp(m_Offset, alignment);

    return alignedOffset + alignedSize <= m_Size;
}

GpuAllocation LinearAllocatorPage::Allocate(size_t size, size_t alignment)
{
    if (!HasSpace(size, alignment))
    {
        LogGraphicsError("An attempt was made to allocate more memory than a linear allocator page allows");
        throw std::bad_alloc();
    }

    size_t alignedSize = AlignUp(size, alignment);
    m_Offset = AlignUp(m_Offset, alignment);

    GpuAllocation allocation(*this);
    allocation.SetSize(alignedSize);
    allocation.SetOffset(m_Offset);
    allocation.SetCpuAddress(static_cast<uint8_t*>(m_CpuAddress) + m_Offset);
    allocation.SetGpuAddress(m_GpuAddress + m_Offset);
        
    return allocation;
}

void LinearAllocatorPage::Reset()
{
    m_Offset = 0;
}

ETH_NAMESPACE_END
