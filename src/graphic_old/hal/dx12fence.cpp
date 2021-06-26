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

#include "dx12fence.h"

ETH_NAMESPACE_BEGIN

DX12Fence::DX12Fence(wrl::ComPtr<ID3D12Device3> device)
{
    ThrowIfFailed(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_Fence))); 
    m_FenceEvent = CreateFenceEvent();
    m_FenceValue = 0;
}

DX12Fence::~DX12Fence()
{
    CloseHandle(m_FenceEvent);
}

HANDLE DX12Fence::CreateFenceEvent() const
{
    HANDLE fenceEvent;
    fenceEvent = CreateEvent(nullptr, false, false, nullptr);
    assert(fenceEvent && "Failed to create fence event");
    return fenceEvent;
}

void DX12Fence::WaitForFenceValue(uint64_t fenceValue)
{
    if (m_Fence->GetCompletedValue() < fenceValue)
    {
        auto maxWaitDuration = chrono::milliseconds::max();
        ThrowIfFailed(m_Fence->SetEventOnCompletion(fenceValue, m_FenceEvent));
        WaitForSingleObject(m_FenceEvent, static_cast<DWORD>(maxWaitDuration.count()));
    }
}

ETH_NAMESPACE_END
