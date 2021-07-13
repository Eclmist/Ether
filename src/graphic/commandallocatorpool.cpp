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

#include "commandallocatorpool.h"

ETH_NAMESPACE_BEGIN

CommandAllocatorPool::CommandAllocatorPool(D3D12_COMMAND_LIST_TYPE type)
    : m_Type(type)
{
}

CommandAllocatorPool::~CommandAllocatorPool()
{
    m_AllocatorPool.clear();
}

ID3D12CommandAllocator* CommandAllocatorPool::RequestAllocator(uint64_t completedFenceValue)
{
    if (m_DiscardedAllocators.empty())
        return CreateNewAllocator();

    if (m_DiscardedAllocators.front().second > completedFenceValue)
        return CreateNewAllocator();

    ID3D12CommandAllocator* allocator = m_DiscardedAllocators.front().first.Get();
    m_DiscardedAllocators.pop();
    ASSERT_SUCCESS(allocator->Reset());
    return allocator;
}

void CommandAllocatorPool::DiscardAllocator(ID3D12CommandAllocator* allocator, uint64_t fenceValue)
{
    m_DiscardedAllocators.emplace(allocator, fenceValue);
}

ID3D12CommandAllocator* CommandAllocatorPool::CreateNewAllocator()
{
    wrl::ComPtr<ID3D12CommandAllocator> allocator;
    ASSERT_SUCCESS(GraphicCore::GetDevice().CreateCommandAllocator(m_Type, IID_PPV_ARGS(&allocator)));
    std::wstring allocatorName = L"CommandAllocatorPool::CommandAllocator" + m_AllocatorPool.size();
    allocator->SetName(allocatorName.c_str());
    m_AllocatorPool.push_back(allocator);
    return allocator.Get();
}

ETH_NAMESPACE_END

