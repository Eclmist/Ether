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

#include "uploadbufferallocator.h"

ETH_NAMESPACE_BEGIN

UploadBufferAllocator::UploadBufferAllocator(size_t pageSize)
    : m_PageSize(pageSize)
    , m_CurrentPage(nullptr)
{
}

UploadBufferAllocation UploadBufferAllocator::Allocate(size_t size, size_t alignment)
{
    if (size > m_PageSize)
    {
        LogGraphicsError("An attempt was made to allocate more memory than a linear allocator allows");
        throw std::bad_alloc();
    }

    if (m_CurrentPage == nullptr)
        m_CurrentPage = &RequestPage();

    if (!m_CurrentPage->HasSpace(size, alignment))
        m_CurrentPage = &RequestPage();

    return m_CurrentPage->Allocate(size, alignment);
}

UploadBufferPage& UploadBufferAllocator::RequestPage()
{
    if (m_AvaliablePages.empty())
    {
        auto newPage = std::make_shared<UploadBufferPage>(m_PageSize);
        m_InFlightPages.emplace_back(newPage);
        return *newPage;
    }

    auto page = m_AvaliablePages.back();
    m_AvaliablePages.pop_back();
    m_InFlightPages.push_back(page);

    AssertGraphics(page != nullptr, "LinearAllocatorPage::RequestPage failed to produce a valid page");
    return *page;
}

void UploadBufferAllocator::Reset()
{
    m_CurrentPage = nullptr;
    m_AvaliablePages = m_InFlightPages;

    for (auto page : m_AvaliablePages)
        page->Reset();
}

ETH_NAMESPACE_END

