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

class DX12Device : public DX12Component<ID3D12Device3>
{
public:
    DX12Device(wrl::ComPtr<IDXGIAdapter4> adapter);

    void CreateDebugInfoQueue();

public:
    inline wrl::ComPtr<ID3D12Device3> Get() override { return m_Device; };

private:
    wrl::ComPtr<ID3D12Device3> m_Device;
    wrl::ComPtr<IDXGIAdapter4> m_Adapter;
    wrl::ComPtr<ID3D12InfoQueue> m_DebugInfoQueue;
};

ETH_NAMESPACE_END