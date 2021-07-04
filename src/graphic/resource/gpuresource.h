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

ETH_NAMESPACE_BEGIN

class GpuResource
{
public:
    GpuResource();
    ~GpuResource() = default;

    inline ID3D12Resource* GetResource() const { return m_Resource.Get(); }
    inline D3D12_GPU_VIRTUAL_ADDRESS GetVirtualAddress() const { return m_GpuAddress; }
    inline D3D12_RESOURCE_STATES GetCurrentState() const { return m_CurrentState; }

    inline void TransitionToState(D3D12_RESOURCE_STATES state) { m_CurrentState = state; }

protected:
    wrl::ComPtr<ID3D12Resource> m_Resource;

    D3D12_GPU_VIRTUAL_ADDRESS m_GpuAddress;
    D3D12_RESOURCE_STATES m_CurrentState;
};

ETH_NAMESPACE_END

