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

ETH_NAMESPACE_BEGIN

class DX12Fence : public DX12Component<ID3D12Fence>
{
public:
    DX12Fence(wrl::ComPtr<ID3D12Device3> device);
    ~DX12Fence();

    HANDLE CreateFenceEvent() const;
    void WaitForFenceValue(uint64_t fenceValue);

public:
    inline wrl::ComPtr<ID3D12Fence> Get() override { return m_Fence; };
    inline uint64_t GetValue() const { return m_FenceValue; };
    inline uint64_t Increment() { return ++m_FenceValue; };

private:
    wrl::ComPtr<ID3D12Fence> m_Fence;
    uint64_t m_FenceValue;
    HANDLE m_FenceEvent;
};

ETH_NAMESPACE_END
