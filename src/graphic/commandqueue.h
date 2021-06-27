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

#include "graphic/commandallocatorpool.h"

ETH_NAMESPACE_BEGIN

class CommandQueue : public NonCopyable
{
public:
    CommandQueue(D3D12_COMMAND_LIST_TYPE type);
    ~CommandQueue() = default;

    inline ID3D12CommandQueue* Get() const { return m_CommandQueue.Get(); }
    inline D3D12_COMMAND_LIST_TYPE GetType() const { return m_Type; }
    
private:
    friend class CommandManager;
    friend class GraphicContext;

    uint64_t CommandQueue::Execute(ID3D12CommandList* cmdLst);

    void InitializeCommandQueue();
    void InitializeFence();

    ID3D12CommandAllocator* RequestAllocator();
    void DiscardAllocator(ID3D12CommandAllocator* allocator);

private:
    const D3D12_COMMAND_LIST_TYPE m_Type;

    wrl::ComPtr<ID3D12CommandQueue> m_CommandQueue;
    wrl::ComPtr<ID3D12Fence> m_Fence;
    uint64_t m_FenceValue; // The *latest* fence value that will be signaled when the command queue is empty.

    CommandAllocatorPool m_AllocatorPool;
};

ETH_NAMESPACE_END