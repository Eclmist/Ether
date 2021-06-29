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

#include "swapchain.h"

ETH_NAMESPACE_BEGIN

void SwapChain::Initialize()
{
    LogGraphicsInfo("Creating D3D12 swapchain");
    m_FrameBufferWidth = g_EngineConfig.GetClientWidth();
    m_FrameBufferHeight = g_EngineConfig.GetClientHeight();
    m_BufferingMode = BufferingMode::BUFFERINGMODE_TRIPLE;
    m_VsyncEnabled = false;
    CreateDxgiSwapChain();
    InitializeResources();
}

void SwapChain::Shutdown()
{
}

void SwapChain::Present()
{
    // TODO: Compute v-blank based on frame time and target framerate
    m_SwapChain->Present(m_VsyncEnabled ? 1 : 0, 0);
    m_CurrentBackBufferIndex = (m_CurrentBackBufferIndex + 1) % GetNumBuffers();
}

std::shared_ptr<TextureResource> SwapChain::GetCurrentBackBuffer() const
{
    return m_FrameBuffers[m_CurrentBackBufferIndex];
}

void SwapChain::CreateDxgiSwapChain()
{
    wrl::ComPtr<IDXGIFactory4> dxgiFactory;
    ASSERT_SUCCESS(CreateDXGIFactory2(0, IID_PPV_ARGS(&dxgiFactory)));

    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.Width = m_FrameBufferWidth;
    swapChainDesc.Height = m_FrameBufferHeight;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.Stereo = FALSE;
    swapChainDesc.SampleDesc = { 1, 0 };
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = GetNumBuffers();
    swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    swapChainDesc.Flags = 0;

    ASSERT_SUCCESS(dxgiFactory->MakeWindowAssociation(Win32::g_hWnd, 0));

    wrl::ComPtr<IDXGISwapChain1> swapChain1;

    ASSERT_SUCCESS(dxgiFactory->CreateSwapChainForHwnd(
        g_CommandManager.GetGraphicsQueue()->Get(),
        Win32::g_hWnd,
        &swapChainDesc,
        nullptr,
        nullptr, // TODO: Test out alt-enter?
        &swapChain1));

    ASSERT_SUCCESS(swapChain1.As(&m_SwapChain));
}

void SwapChain::InitializeResources()
{
    for (uint32_t i = 0; i < GetNumBuffers(); ++i)
    {
        wrl::ComPtr<ID3D12Resource> framebuffer;
        ASSERT_SUCCESS(m_SwapChain->GetBuffer(i, IID_PPV_ARGS(&framebuffer)));
        m_FrameBuffers[i] = std::make_shared<TextureResource>(L"Primary Frame Buffer", framebuffer.Detach());
    }

    m_CurrentBackBufferIndex = m_SwapChain->GetCurrentBackBufferIndex();
}

ETH_NAMESPACE_END

