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

#include "graphic/resource/virtualbuffer/linearallocator.h"

ETH_NAMESPACE_BEGIN

class CommandQueue;

class CommandContext : public NonCopyable
{
public:
    CommandContext(D3D12_COMMAND_LIST_TYPE type = D3D12_COMMAND_LIST_TYPE_DIRECT);
    ~CommandContext();
    void Reset();

    CommandQueue& GetCommandQueue() const;
    uint64_t GetCompletionFenceValue() const;

public:
    inline ID3D12GraphicsCommandList& GetCommandList() const { return *m_CommandList.Get(); }

public:
    void TransitionResource(GpuResource& resource, D3D12_RESOURCE_STATES newState);
    void CopyBufferRegion(GpuResource& dest, GpuResource& src, size_t size, size_t dstOffset = 0);
    void FinalizeAndExecute(bool waitForCompletion = false);

public:
    static void InitializeBuffer(GpuResource& dest, const void* data, size_t size, size_t dstOffset = 0);

protected:
    wrl::ComPtr<ID3D12GraphicsCommandList> m_CommandList;
    ID3D12CommandAllocator* m_CommandAllocator;
    D3D12_COMMAND_LIST_TYPE m_Type;
    LinearAllocator m_GpuAllocator;
};

ETH_NAMESPACE_END

