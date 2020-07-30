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

#include "dx12commandqueue.h"

DX12CommandQueue::DX12CommandQueue(
    wrl::ComPtr<ID3D12Device3> device,
    D3D12_COMMAND_LIST_TYPE type,
    D3D12_COMMAND_QUEUE_PRIORITY priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL,
    D3D12_COMMAND_QUEUE_FLAGS flags = D3D12_COMMAND_QUEUE_FLAG_NONE)
    : m_Type(type)
    , m_Priority(priority)
{
    D3D12_COMMAND_QUEUE_DESC desc;
    
    desc.Type = type;
    desc.Priority = priority;
    desc.Flags = flags;
    desc.NodeMask = 0;

    ThrowIfFailed(device->CreateCommandQueue(&desc, IID_PPV_ARGS(&m_CommandQueue)));
}

uint64_t DX12CommandQueue::Signal(DX12Fence& fence)
{
    wrl::ComPtr<ID3D12Fence> halFence = fence.Get();
    ThrowIfFailed(m_CommandQueue->Signal(halFence.Get(), fence.Increment()));
    return fence.GetValue();
}
