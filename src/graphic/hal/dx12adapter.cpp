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

#include "dx12adapter.h"

DX12Adapter::DX12Adapter(bool useWarp)
{
    m_Adapter = QueryAdapter(useWarp);
}

wrl::ComPtr<IDXGIAdapter4> DX12Adapter::QueryAdapter(bool useWarp)
{
    wrl::ComPtr<IDXGIFactory4> dxgiFactory = CreateDxgiFactory();

    if (useWarp)
        return QueryWARPAdapter(dxgiFactory);
    else
        return QueryHardwareAdapter(dxgiFactory);
}

wrl::ComPtr<IDXGIAdapter4> DX12Adapter::QueryWARPAdapter(wrl::ComPtr<IDXGIFactory4> dxgiFactory)
{
    wrl::ComPtr<IDXGIAdapter1> dxgiAdapter1;
    wrl::ComPtr<IDXGIAdapter4> dxgiAdapter4;

    ThrowIfFailed(dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&dxgiAdapter1)));
    ThrowIfFailed(dxgiAdapter1.As(&dxgiAdapter4));

    return dxgiAdapter4;
}

wrl::ComPtr<IDXGIAdapter4> DX12Adapter::QueryHardwareAdapter(wrl::ComPtr<IDXGIFactory4> dxgiFactory)
{
    wrl::ComPtr<IDXGIAdapter1> dxgiAdapter1;
    wrl::ComPtr<IDXGIAdapter4> dxgiAdapter4;

    SIZE_T maxDedicatedVideoMemory = 0;
    for (UINT i = 0; dxgiFactory->EnumAdapters1(i, &dxgiAdapter1) != DXGI_ERROR_NOT_FOUND; ++i)
    {
        DXGI_ADAPTER_DESC1 dxgiAdapterDesc1;
        dxgiAdapter1->GetDesc1(&dxgiAdapterDesc1);

        // Check if adapter is actually a hardware adapter
        if ((dxgiAdapterDesc1.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) != 0)
            continue;

        // Check if the DX12 device can be created
        if (FAILED(D3D12CreateDevice(dxgiAdapter1.Get(), D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr)))
            continue;

        // Check if this device has the largest vram so far. Use vram as a indicator of perf for now
        if (dxgiAdapterDesc1.DedicatedVideoMemory <= maxDedicatedVideoMemory)
            continue;

        maxDedicatedVideoMemory = dxgiAdapterDesc1.DedicatedVideoMemory;
        ThrowIfFailed(dxgiAdapter1.As(&dxgiAdapter4));
    }

    return dxgiAdapter4;
}
