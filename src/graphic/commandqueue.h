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
    inline uint64_t GetCompletionFenceValue() const { return m_CompletionFenceValue; }
    inline uint64_t GetCompletedFenceValue() const { return m_Fence->GetCompletedValue(); }

    bool IsFenceComplete(uint64_t fenceValue);
    void StallForFence(uint64_t fenceValue);
    void Flush();
    
private:
    friend class CommandManager;

    uint64_t Execute(ID3D12CommandList* cmdLst);

private:
    void InitializeCommandQueue();
    void InitializeFence();

    ID3D12CommandAllocator& RequestAllocator();
    void DiscardAllocator(ID3D12CommandAllocator& allocator, uint64_t fenceValue);

private:
    const D3D12_COMMAND_LIST_TYPE m_Type;

    wrl::ComPtr<ID3D12CommandQueue> m_CommandQueue;
    wrl::ComPtr<ID3D12Fence> m_Fence;
    uint64_t m_CompletionFenceValue;
    uint64_t m_LastKnownFenceValue;

    const std::unique_ptr<CommandAllocatorPool> m_AllocatorPool;

    HANDLE m_FenceEventHandle;
};

ETH_NAMESPACE_END