/*
    This file is part of Ether, an open-source DirectX 12 renderer.

    Copyright (c) 2020-2022 Samuel Huang - All rights reserved.

    Ether is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "d3d12module.h"
#include "d3d12device.h"

ETH_NAMESPACE_BEGIN

RHIResult D3D12Module::Initialize()
{
    InitializeDebugLayer();
	RHIResult result = InitializeAdapter();

    return result;
}

RHIResult D3D12Module::Shutdown()
{
    return RHIResult::Success;
}

RHIResult D3D12Module::CreateDevice(RHIDeviceHandle& device) const
{
    D3D12Device* d3d12Device = new D3D12Device();
    device.Set(d3d12Device);

    HRESULT hr = D3D12CreateDevice(m_Adapter.Get(), D3D12_MINIMUM_FEATURE_LEVEL, IID_PPV_ARGS(&d3d12Device->m_Device));
    return FAILED(hr) ? RHIResult::Failure : RHIResult::Success;
}

RHIResult D3D12Module::InitializeAdapter()
{
	wrl::ComPtr<IDXGIAdapter1> dxgiAdapter1;

    wrl::ComPtr<IDXGIFactory4> dxgiFactory;
    ASSERT_SUCCESS(CreateDXGIFactory2(0, IID_PPV_ARGS(&dxgiFactory)));

    bool useWarp = false;

    if (useWarp)
    {
        HRESULT hr = dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&m_Adapter));
        return TO_RHI_RESULT(hr);
    }

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
        ASSERT_SUCCESS(dxgiAdapter1.As(&m_Adapter));
        return RHIResult::Success;
    }

    return RHIResult::Failure;
}

RHIResult D3D12Module::InitializeDebugLayer()
{
#if defined(_DEBUG)
    wrl::ComPtr<ID3D12Debug> debugInterface;
    ASSERT_SUCCESS(D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface)));
    debugInterface->EnableDebugLayer();
#endif
    return RHIResult::Success;
}

ETH_NAMESPACE_END

