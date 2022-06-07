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

#include "commandallocatorpool.h"
#include "graphic/rhi/rhicommandallocator.h"
#include "graphic/rhi/rhidevice.h"

ETH_NAMESPACE_BEGIN

CommandAllocatorPool::CommandAllocatorPool(RhiCommandListType type)
    : m_Type(type)
{
}

CommandAllocatorPool::~CommandAllocatorPool()
{
    for (auto allocator : m_AllocatorPool)
        allocator.Destroy();

    m_AllocatorPool.clear();
}

RhiCommandAllocatorHandle CommandAllocatorPool::RequestAllocator(RhiFenceValue completedFenceValue)
{
    if (m_DiscardedAllocators.empty())
        return CreateNewAllocator();

    if (m_DiscardedAllocators.front().second > completedFenceValue)
        return CreateNewAllocator();

    RhiCommandAllocatorHandle allocator = m_DiscardedAllocators.front().first;
    m_DiscardedAllocators.pop();
    ASSERT_SUCCESS(allocator->Reset());
    return allocator;
}

void CommandAllocatorPool::DiscardAllocator(RhiCommandAllocatorHandle allocator, RhiFenceValue fenceValue)
{
    m_DiscardedAllocators.emplace(allocator, fenceValue);
}

RhiCommandAllocatorHandle CommandAllocatorPool::CreateNewAllocator()
{
    RhiCommandAllocatorHandle allocator;
    allocator.SetName(L"CommandAllocatorPool::CommandAllocator");

    ASSERT_SUCCESS(GraphicCore::GetDevice()->CreateCommandAllocator({ m_Type }, allocator));
    m_AllocatorPool.push_back(allocator);

    return allocator;
}

ETH_NAMESPACE_END

