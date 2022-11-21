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
#include "graphics/commandallocatorpool.h"
#include "graphics/rhi/rhicommandallocator.h"
#include "graphics/rhi/rhidevice.h"

Ether::Graphics::CommandAllocatorPool::CommandAllocatorPool(RhiCommandType type)
    : m_Type(type)
{
}

Ether::Graphics::RhiCommandAllocator& Ether::Graphics::CommandAllocatorPool::RequestAllocator(RhiFenceValue completedFenceValue)
{
    if (m_DiscardedAllocators.empty())
        return CreateNewAllocator();

    if (m_DiscardedAllocators.front().second > completedFenceValue)
        return CreateNewAllocator();

    RhiCommandAllocator& allocator = m_DiscardedAllocators.front().first;
    m_DiscardedAllocators.pop();
    allocator.Reset();
    return allocator;
}

void Ether::Graphics::CommandAllocatorPool::DiscardAllocator(RhiCommandAllocator& allocator, RhiFenceValue fenceValue)
{
    m_DiscardedAllocators.emplace(allocator, fenceValue);
}

Ether::Graphics::RhiCommandAllocator& Ether::Graphics::CommandAllocatorPool::CreateNewAllocator()
{
    m_AllocatorPool.emplace_back(Core::GetDevice().CreateCommandAllocator({ m_Type }));
    return *m_AllocatorPool.back();
}

