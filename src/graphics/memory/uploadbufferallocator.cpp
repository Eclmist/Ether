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

#include "graphics/memory/uploadbufferallocator.h"

Ether::Graphics::UploadBufferAllocator::UploadBufferAllocator(size_t pageSize)
    : MemoryAllocator(std::numeric_limits<size_t>::max()) // No upper bound on max number of pages
    , m_PageSize(pageSize)
{
    m_CurrentPage = GetNextAvailablePage();
}

std::unique_ptr<Ether::MemoryAllocation> Ether::Graphics::UploadBufferAllocator::Allocate(SizeAlign sizeAlign)
{
    if (AlignUp(sizeAlign.m_Size, sizeAlign.m_Alignment) > m_PageSize)
        return nullptr;

    if (!m_CurrentPage->HasSpace(sizeAlign))
        m_CurrentPage = GetNextAvailablePage();

    return m_CurrentPage->Allocate(sizeAlign);
}

void Ether::Graphics::UploadBufferAllocator::Free(std::unique_ptr<MemoryAllocation>&& alloc)
{
    // Page is implemented as a linear allocator which does not support freeing.
    // UploadBufferAllocation& alloc = dynamic_cast<UploadBufferAllocation&>(*alloc);
    // alloc->GetAllocator()->Free(alloc);
}

bool Ether::Graphics::UploadBufferAllocator::HasSpace(SizeAlign sizeAlign) const
{
    return m_CurrentPage->HasSpace(sizeAlign) || m_PagePool.size() >= m_Capacity;
}

void Ether::Graphics::UploadBufferAllocator::Reset()
{
    m_AvaliablePages.clear();

    for (auto& page : m_PagePool)
    {
        page->Reset();
        m_AvaliablePages.push_back(page.get());
    }
}

Ether::Graphics::UploadBufferAllocatorPage* Ether::Graphics::UploadBufferAllocator::GetNextAvailablePage()
{
    if (m_AvaliablePages.empty())
        AllocatePage();

    UploadBufferAllocatorPage* nextPage = m_AvaliablePages.back();
    m_AvaliablePages.pop_back();

    return nextPage;
}

void Ether::Graphics::UploadBufferAllocator::AllocatePage()
{
    if (m_PagePool.size() >= m_Capacity)
        throw std::bad_alloc();

    m_PagePool.emplace_back(std::make_unique<UploadBufferAllocatorPage>(m_PageSize));
    m_AvaliablePages.push_back(m_PagePool.back().get());
}
