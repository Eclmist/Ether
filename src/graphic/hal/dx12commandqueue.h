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
#include "graphic/hal/dx12fence.h"

ETH_NAMESPACE_BEGIN

class DX12CommandQueue : public DX12Component<ID3D12CommandQueue>
{
public:
    DX12CommandQueue(
        wrl::ComPtr<ID3D12Device3> device,
        D3D12_COMMAND_LIST_TYPE type);

public:
    void Signal(DX12Fence& fence, uint64_t fenceValue);

public:
    inline wrl::ComPtr<ID3D12CommandQueue> Get() override { return m_CommandQueue; };
    inline D3D12_COMMAND_LIST_TYPE GetType() const { return m_Type; };
    inline D3D12_COMMAND_QUEUE_PRIORITY GetPriority() const { return m_Priority; };

private:
    wrl::ComPtr<ID3D12CommandQueue> m_CommandQueue;

    D3D12_COMMAND_LIST_TYPE m_Type;
    D3D12_COMMAND_QUEUE_PRIORITY m_Priority;
};

ETH_NAMESPACE_END
