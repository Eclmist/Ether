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

#include "gfxlinearallocator.h"
#include "graphic/virtualbuffers/gfxmemoryutils.h"

GfxLinearAllocator::GfxLinearAllocator(
    wrl::ComPtr<ID3D12Device3> device,
    size_t pageSize)
    : m_Device(device)
    , m_PageSize(pageSize)
{
}

GfxLinearAllocator::~GfxLinearAllocator()
{

}

GfxAllocation GfxLinearAllocator::Allocate(size_t sizeInBytes, size_t alignment)
{
    if (sizeInBytes > m_PageSize)
        throw std::bad_alloc();

    if (!m_CurrentPage || !m_CurrentPage->HasSpace(sizeInBytes, alignment))
        m_CurrentPage = RequestNewPage();

    return m_CurrentPage->Allocate(sizeInBytes, alignment);
}

void GfxLinearAllocator::Reset()
{
    m_CurrentPage = nullptr;
    m_AvailablePages = m_PagePool;

    for (int i = 0; i < m_AvailablePages.size(); ++i)
        m_AvailablePages[i]->Reset();
}

std::shared_ptr<GfxMemoryPage> GfxLinearAllocator::RequestNewPage()
{
    std::shared_ptr<GfxMemoryPage> page;

    if (!m_AvailablePages.empty())
    {
        page = m_AvailablePages.front();
        m_AvailablePages.pop_front();
    }
    else
    {
        page = std::make_shared<GfxMemoryPage>(m_Device, m_PageSize);
        m_PagePool.push_back(page);
    }

    return page;
}
