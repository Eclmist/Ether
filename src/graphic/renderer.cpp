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

#include "renderer.h"

ETH_NAMESPACE_BEGIN

wrl::ComPtr<ID3D12Device3> g_GraphicDevice;

void Renderer::Initialize()
{
    if (g_GraphicDevice != nullptr)
    {
        LogGraphicsWarning("An attempt was made to initialize an already initialized Renderer");
        return;
    }

    LogGraphicsInfo("Initializing Renderer");

    InitializeAdapter();
    InitializeDevice();
    InitializeCommandManager();
    InitializeSwapChain();
}

void Renderer::Shutdown()
{

}

void Renderer::Render()
{
    
}

void Renderer::InitializeAdapter()
{
    wrl::ComPtr<IDXGIAdapter1> dxgiAdapter1;

    wrl::ComPtr<IDXGIFactory4> dxgiFactory;
    ASSERT_SUCCESS(CreateDXGIFactory2(0, IID_PPV_ARGS(&dxgiFactory)), DXGIFactory);

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
        ASSERT_SUCCESS(dxgiAdapter1.As(&m_Adapter), DXGIAdapter);
    }
}

void Renderer::InitializeDevice()
{
    ASSERT_SUCCESS(D3D12CreateDevice(m_Adapter.Get(), ETH_MINIMUM_FEATURE_LEVEL, IID_PPV_ARGS(&g_GraphicDevice)), GraphicDevice);
}

void Renderer::InitializeCommandManager()
{
    m_CommandManager = std::make_shared<CommandManager>();
}

void Renderer::InitializeSwapChain()
{
    LogGraphicsInfo("Creating D3D12 swapchain");

    wrl::ComPtr<IDXGIFactory4> dxgiFactory;
    ASSERT_SUCCESS(CreateDXGIFactory2(0, IID_PPV_ARGS(&dxgiFactory)), DXGIFactory);

    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.Width = m_FrameBufferWidth;
    swapChainDesc.Height = m_FrameBufferHeight;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.Stereo = FALSE;
    swapChainDesc.SampleDesc = { 1, 0 };
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = ETH_NUM_SWAPCHAIN_BUFFERS;
    swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    swapChainDesc.Flags = 0;

    ASSERT_SUCCESS(dxgiFactory->MakeWindowAssociation(Win32::g_hWnd, 0), 
        "window association with DXGI Factory");

    wrl::ComPtr<IDXGISwapChain1> swapChain1;

    ASSERT_SUCCESS(dxgiFactory->CreateSwapChainForHwnd(
        m_CommandManager->GetGraphicsQueue(),
        Win32::g_hWnd,
        &swapChainDesc,
        nullptr,
        nullptr, // TODO: Test out alt-enter?
        &swapChain1), SwapChain1);

    ASSERT_SUCCESS(swapChain1.As(&m_SwapChain), SwapChain4);
}

ETH_NAMESPACE_END
