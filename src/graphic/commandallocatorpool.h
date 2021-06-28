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

ETH_NAMESPACE_BEGIN

class CommandAllocatorPool : public NonCopyable
{
public:
    CommandAllocatorPool(D3D12_COMMAND_LIST_TYPE type);
    ~CommandAllocatorPool();

    ID3D12CommandAllocator* RequestAllocator(uint64_t completedFenceValue);
    void DiscardAllocator(ID3D12CommandAllocator* allocator, uint64_t fenceValue);

private:
    ID3D12CommandAllocator* CreateNewAllocator();
    
private:
    const D3D12_COMMAND_LIST_TYPE m_Type;

    std::vector<wrl::ComPtr<ID3D12CommandAllocator>> m_AllocatorPool;
    std::queue<std::pair<wrl::ComPtr<ID3D12CommandAllocator>, uint64_t>> m_DiscardedAllocators;
};

ETH_NAMESPACE_END