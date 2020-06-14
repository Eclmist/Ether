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

#include "graphic/hal/dx12device.h"

void DX12Device::CreateDevice()
{
    if (m_Device != nullptr)
        return;

    ThrowIfFailed(D3D12CreateDevice(m_Adapter.Get(), ETH_MINIMUM_FEATURE_LEVEL, IID_PPV_ARGS(&m_Device)));
}

void DX12Device::CreateDebugInfoQueue()
{
    ThrowIfFailed(m_Device.As(&m_DebugInfoQueue));

    D3D12_MESSAGE_SEVERITY breakSeverity[] = {
        D3D12_MESSAGE_SEVERITY_CORRUPTION,
        D3D12_MESSAGE_SEVERITY_ERROR,
        D3D12_MESSAGE_SEVERITY_WARNING
    };

    // Suppress messages based on their severity level
    D3D12_MESSAGE_SEVERITY suppressSeverity[] = {
        D3D12_MESSAGE_SEVERITY_INFO
    };

    // Suppress individual messages by their ID
    D3D12_MESSAGE_ID suppressID[] = {
        D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE, // Allow arbitrary clear colors
    };


    D3D12_INFO_QUEUE_FILTER NewFilter = {};
    NewFilter.DenyList.NumSeverities = _countof(breakSeverity);
    NewFilter.DenyList.pSeverityList = suppressSeverity;
    NewFilter.DenyList.NumIDs = _countof(suppressID);
    NewFilter.DenyList.pIDList = suppressID;

    ThrowIfFailed(m_DebugInfoQueue->PushStorageFilter(&NewFilter));
}

