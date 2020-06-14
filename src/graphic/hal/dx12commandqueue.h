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

#include "graphic/hal/dx12component.h"

class DX12CommandQueue : public DX12Component<ID3D12CommandQueue>
{
public:
    DX12CommandQueue(
        wrl::ComPtr<ID3D12Device3> device,
        D3D12_COMMAND_LIST_TYPE type,
        D3D12_COMMAND_QUEUE_PRIORITY priority,
        D3D12_COMMAND_QUEUE_FLAGS flags)
        : m_Device(device)
        , m_Type(type)
        , m_Priority(priority)
        , m_Flags(flags) {};
    
    void CreateCommandQueue();

public:
    inline wrl::ComPtr<ID3D12CommandQueue> Get() override { return m_CommandQueue; };

private:
    wrl::ComPtr<ID3D12CommandQueue> m_CommandQueue;
    wrl::ComPtr<ID3D12Device3> m_Device;

    D3D12_COMMAND_LIST_TYPE m_Type;
    D3D12_COMMAND_QUEUE_PRIORITY m_Priority;
    D3D12_COMMAND_QUEUE_FLAGS m_Flags;
};

