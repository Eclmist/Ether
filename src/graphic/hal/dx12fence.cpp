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

#include "dx12fence.h"

DX12Fence::DX12Fence(wrl::ComPtr<ID3D12Device3> device)
    : m_FenceValue(0)
{
    ThrowIfFailed(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_Fence))); 
    m_FenceEvent = CreateFenceEvent();
}

HANDLE DX12Fence::CreateFenceEvent() const noexcept
{
    HANDLE fenceEvent;
    fenceEvent = CreateEvent(nullptr, false, false, nullptr);
    assert(fenceEvent && "Failed to create fence event");
    return fenceEvent;
}

void DX12Fence::WaitForFence()
{
    if (m_Fence->GetCompletedValue() < m_FenceValue)
    {
// TODO: Why the fuck is max still defined here?
#if defined(max)
#undef max
#endif 
        auto maxWaitDuration = chrono::milliseconds::max();
        ThrowIfFailed(m_Fence->SetEventOnCompletion(m_FenceValue, m_FenceEvent));
        WaitForSingleObject(m_FenceEvent, static_cast<DWORD>(maxWaitDuration.count()));
    }
}

void DX12Fence::Release()
{
    CloseHandle(m_FenceEvent);
}
