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
    if (m_IsInitialized)
    {
        LogGraphicsWarning("An attempt was made to initialize an already initialized Renderer");
        return;
    }

    InitializeDevice();
    InitializeSwapChain();

    m_IsInitialized = true;
}

void Renderer::Shutdown()
{

}

void Renderer::Render()
{

}

void Renderer::InitializeDevice()
{


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
        g_CommandManager.GetGraphicsQueue(),
        Win32::g_hWnd,
        &swapChainDesc,
        nullptr,
        nullptr, // TODO: Test out alt-enter?
        &swapChain1), SwapChain1);

    ASSERT_SUCCESS(swapChain1.As(&m_SwapChain), SwapChain4);
}

ETH_NAMESPACE_END
