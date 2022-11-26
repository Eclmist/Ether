/*
    This file is part of Ether, an open-source DirectX 12 renderer.

    Copyright (c) 2020-2023 Samuel Huang - All rights reserved.

    Ether is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#ifdef ETH_GRAPHICS_DX12

#include "graphics/core.h"
#include "graphics/rhi/dx12/dx12module.h"
#include "graphics/rhi/dx12/dx12device.h"
#include <dxgidebug.h>

Ether::Graphics::Dx12Module::Dx12Module()
{
    LogGraphicsInfo("Initializing DirectX 12");

    if (Core::GetConfig().m_UseValidationLayer)
        InitializeDebugLayer();

    InitializeFactory();
    InitializeAdapter();
}

Ether::Graphics::Dx12Module::~Dx12Module()
{
    if (Core::GetConfig().m_UseValidationLayer)
        ReportLiveObjects();
}

std::unique_ptr<Ether::Graphics::RhiDevice> Ether::Graphics::Dx12Module::CreateDevice() const
{
    std::unique_ptr<Dx12Device> dx12Device = std::make_unique<Dx12Device>();
    HRESULT hr = D3D12CreateDevice(m_Adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&dx12Device->m_Device));
    if (FAILED(hr))
        LogGraphicsFatal("Failed to create DX12 Device");

    return dx12Device;
}

void Ether::Graphics::Dx12Module::InitializeFactory()
{
    HRESULT hr = CreateDXGIFactory2(0, IID_PPV_ARGS(&m_DxgiFactory));
    if (FAILED(hr))
        LogGraphicsFatal("Failed to create DXGI Factory");
}

void Ether::Graphics::Dx12Module::InitializeAdapter()
{
    wrl::ComPtr<IDXGIAdapter1> dxgiAdapter1;

    //bool useWarp = false;

    //if (useWarp)
    //{
    //    HRESULT hr = m_DxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&m_Adapter));
    //    if (FAILED(hr))
    //        LogGraphicsError("Failed to enumerate Warp adapters");
    //}

    SIZE_T maxDedicatedVideoMemory = 0;
    for (UINT i = 0; m_DxgiFactory->EnumAdapters1(i, &dxgiAdapter1) != DXGI_ERROR_NOT_FOUND; ++i)
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
        HRESULT hr = dxgiAdapter1.As(&m_Adapter);
        if (FAILED(hr))
            LogGraphicsFatal("Failed to create DX12 Adapter");
    }
}

void Ether::Graphics::Dx12Module::InitializeDebugLayer()
{
#if defined(_DEBUG)
    LogGraphicsInfo("Graphics Debug Layer is enabled");
    wrl::ComPtr<ID3D12Debug> debugInterface;
    HRESULT hr = D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface));
    if (FAILED(hr))
        LogGraphicsWarning("Failed to create DX12 debug interface");
    else
        debugInterface->EnableDebugLayer();
#endif
}

void Ether::Graphics::Dx12Module::ReportLiveObjects()
{
#if defined(_DEBUG)
    IDXGIDebug1* pDebug = NULL;
    if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&pDebug))))
    {
        pDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_SUMMARY);
        pDebug->Release();
    }
#endif
}

#endif // ETH_GRAPHICS_DX12

