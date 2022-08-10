/*
    This file is part of Ether, an open-source DirectX 12 renderer.

    Copyright (c) 2020-2022 Samuel Huang - All rights reserved.

    Ether is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "descriptorallocator.h"
#include "descriptorallocatorpage.h"

ETH_NAMESPACE_BEGIN

// Arbitrary max page size
constexpr uint32_t MaxDescriptorsPerHeap = 256;

DescriptorAllocator::DescriptorAllocator(RhiDescriptorHeapType type) 
    : m_HeapType(type)
{
}

std::shared_ptr<DescriptorAllocation> DescriptorAllocator::Allocate(uint32_t numDescriptors)
{
	std::lock_guard<std::mutex> guard(m_AllocationMutex);
	std::shared_ptr<DescriptorAllocatorPage> pageWithEnoughSpace;

	if (numDescriptors > MaxDescriptorsPerHeap)
	{
		LogGraphicsFatal("An attempt was made to allocate more descriptors (%d) than allowed (%d)", numDescriptors, MaxDescriptorsPerHeap);
		throw std::bad_alloc();
	}

    auto availablePage = GetAvailablePages(numDescriptors);

    if (availablePage == nullptr)
        availablePage = CreateAllocatorPage();

	return availablePage->Allocate(numDescriptors);
}

void DescriptorAllocator::ReleaseStaleDescriptors(uint64_t frameNumber)
{
    std::lock_guard<std::mutex> guard(m_AllocationMutex);

    for (size_t i = 0; i < m_AllocatorPagePool.size(); ++i)
    {
        auto page = m_AllocatorPagePool[i];
        page->ReleaseStaleDescriptors(frameNumber);
        if (page->GetNumFreeHandles() > 0)
            m_AvailablePageIndices.insert(i);
    }
}

std::shared_ptr<DescriptorAllocatorPage> DescriptorAllocator::CreateAllocatorPage()
{
	auto newPage = std::make_shared<DescriptorAllocatorPage>(
        m_HeapType,
        MaxDescriptorsPerHeap);

	m_AllocatorPagePool.emplace_back(newPage);
	m_AvailablePageIndices.insert(m_AllocatorPagePool.size() - 1);
	return newPage;
}

std::shared_ptr<DescriptorAllocatorPage> DescriptorAllocator::GetAvailablePages(uint32_t numDescriptors)
{
	for (auto iter = m_AvailablePageIndices.begin(); iter != m_AvailablePageIndices.end(); ++iter)
	{
		auto allocatorPage = m_AllocatorPagePool[*iter];
        if (allocatorPage->HasSpace(numDescriptors))
            return allocatorPage;

		if (m_AllocatorPagePool[*iter]->GetNumFreeHandles() <= 0)
			iter = m_AvailablePageIndices.erase(iter);
	}

    return nullptr;
}

ETH_NAMESPACE_END

