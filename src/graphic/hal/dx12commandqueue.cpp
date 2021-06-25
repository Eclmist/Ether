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

#include "dx12commandqueue.h"

ETH_NAMESPACE_BEGIN

DX12CommandQueue::DX12CommandQueue(
    wrl::ComPtr<ID3D12Device3> device,
    D3D12_COMMAND_LIST_TYPE type)
    : m_Type(type)
    , m_Priority(D3D12_COMMAND_QUEUE_PRIORITY_NORMAL)
{
    D3D12_COMMAND_QUEUE_DESC desc;
    
    desc.Type = type;
    desc.Priority = m_Priority;
    desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    desc.NodeMask = 0;

    ThrowIfFailed(device->CreateCommandQueue(&desc, IID_PPV_ARGS(&m_CommandQueue)));
}

void DX12CommandQueue::Signal(DX12Fence& fence, uint64_t fenceValue)
{
    wrl::ComPtr<ID3D12Fence> halFence = fence.Get();
    ThrowIfFailed(m_CommandQueue->Signal(halFence.Get(), fenceValue));
}

ETH_NAMESPACE_END
