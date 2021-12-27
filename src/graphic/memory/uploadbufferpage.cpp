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

#include "uploadbufferpage.h"
#include "memoryutils.h"

#include "graphic/rhi/rhidevice.h"
#include "graphic/rhi/rhiresource.h"

ETH_NAMESPACE_BEGIN

UploadBufferPage::UploadBufferPage(size_t size)
    : m_Size(size)
    , m_Offset(0)
    , m_CpuAddress(nullptr)
{
    RHICommitedResourceDesc desc = {};
    desc.m_HeapType = RHIHeapType::Upload;
    desc.m_State = RHIResourceState::GenericRead;
    desc.m_Size = size;

    ASSERT_SUCCESS(GraphicCore::GetDevice()->CreateCommittedResource(desc, m_Resource));

    m_Resource->Map(&m_CpuAddress);
    m_Resource->SetName(L"LinearAllocator::LinearAllocatorPage");
}

UploadBufferPage::~UploadBufferPage()
{
    m_Resource->Unmap();
}

bool UploadBufferPage::HasSpace(size_t size, size_t alignment)
{
    size_t alignedSize = AlignUp(size, alignment);
    size_t alignedOffset = AlignUp(m_Offset, alignment);

    return alignedOffset + alignedSize <= m_Size;
}

UploadBufferAllocation UploadBufferPage::Allocate(size_t size, size_t alignment)
{
    if (!HasSpace(size, alignment))
    {
        LogGraphicsError("An attempt was made to allocate more memory than a linear allocator page allows");
        throw std::bad_alloc();
    }

    size_t alignedSize = AlignUp(size, alignment);
    m_Offset = AlignUp(m_Offset, alignment);

    UploadBufferAllocation allocation(*this);
    allocation.SetSize(alignedSize);
    allocation.SetOffset(m_Offset);
    allocation.SetMappedCPUAddress(static_cast<uint8_t*>(m_CpuAddress) + m_Offset);

    // TODO: do we need store an offset per allocation? Should be uploading the entire page in one go?
    //allocation.SetGPU(m_Resource->GetGPUVirtualAdd + m_Offset);
        
    return allocation;
}

void UploadBufferPage::Reset()
{
    m_Offset = 0;
}

ETH_NAMESPACE_END
