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

#include "graphicdisplay.h"

ETH_NAMESPACE_BEGIN

DXGI_FORMAT g_SwapChainFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

void GraphicDisplay::Initialize()
{
    m_FrameBufferWidth = g_EngineConfig.GetClientWidth();
    m_FrameBufferHeight = g_EngineConfig.GetClientHeight();
    m_BufferingMode = BufferingMode::BUFFERINGMODE_TRIPLE;
    m_ScissorRect = CD3DX12_RECT(0, 0, LONG_MAX, LONG_MAX);
    m_Viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, (float)m_FrameBufferWidth, (float)m_FrameBufferHeight);
    m_VsyncEnabled = false;

    CreateDxgiSwapChain();
    InitializeResources();
}

void GraphicDisplay::Shutdown()
{
    m_SwapChain.Reset();
}

void GraphicDisplay::Present()
{
    // TODO: Compute v-blank based on frame time and target framerate
    m_SwapChain->Present(m_VsyncEnabled ? 1 : 0, 0);
    m_CurrentBackBufferIndex = (m_CurrentBackBufferIndex + 1) % GetNumBuffers();
}

void GraphicDisplay::Resize(uint32_t width, uint32_t height)
{
    if (m_FrameBufferWidth == width && m_FrameBufferHeight == height)
        return;

    m_FrameBufferWidth = width;
    m_FrameBufferHeight = height;
    m_Viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, (float)m_FrameBufferWidth, (float)m_FrameBufferHeight);

    g_CommandManager.Flush();

    for (uint32_t i = 0; i < GetNumBuffers(); ++i)
        m_FrameBuffers[i].reset();

    m_DepthBuffer.reset();

    DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
    ASSERT_SUCCESS(m_SwapChain->GetDesc(&swapChainDesc));
    ASSERT_SUCCESS(m_SwapChain->ResizeBuffers(
        GetNumBuffers(),
        m_FrameBufferWidth,
        m_FrameBufferHeight,
        swapChainDesc.BufferDesc.Format,
        swapChainDesc.Flags));

    m_CurrentBackBufferIndex = m_SwapChain->GetCurrentBackBufferIndex();
    InitializeResources();
}

std::shared_ptr<TextureResource> GraphicDisplay::GetCurrentBackBuffer() const
{
    return m_FrameBuffers[m_CurrentBackBufferIndex];
}

std::shared_ptr<DepthStencilResource> GraphicDisplay::GetDepthBuffer() const
{
    return m_DepthBuffer;
}

void GraphicDisplay::CreateDxgiSwapChain()
{
    wrl::ComPtr<IDXGIFactory4> dxgiFactory;
    ASSERT_SUCCESS(CreateDXGIFactory2(0, IID_PPV_ARGS(&dxgiFactory)));

    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.Width = m_FrameBufferWidth;
    swapChainDesc.Height = m_FrameBufferHeight;
    swapChainDesc.Format = g_SwapChainFormat;
    swapChainDesc.Stereo = FALSE;
    swapChainDesc.SampleDesc = { 1, 0 };
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = GetNumBuffers();
    swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    swapChainDesc.Flags = 0;

    ASSERT_SUCCESS(dxgiFactory->MakeWindowAssociation(Win32::g_hWnd, DXGI_MWA_NO_ALT_ENTER));

    wrl::ComPtr<IDXGISwapChain1> swapChain1;

    ASSERT_SUCCESS(dxgiFactory->CreateSwapChainForHwnd(
        g_CommandManager.GetGraphicsQueue().Get(),
        Win32::g_hWnd,
        &swapChainDesc,
        nullptr,
        nullptr,
        &swapChain1));

    ASSERT_SUCCESS(swapChain1.As(&m_SwapChain));
}

void GraphicDisplay::InitializeResources()
{
    for (uint32_t i = 0; i < GetNumBuffers(); ++i)
    {
        wrl::ComPtr<ID3D12Resource> framebuffer;
        ASSERT_SUCCESS(m_SwapChain->GetBuffer(i, IID_PPV_ARGS(&framebuffer)));
        m_FrameBuffers[i] = std::make_shared<TextureResource>(L"Primary Frame Buffer", framebuffer.Detach());
    }

    m_DepthBuffer = std::make_shared<DepthStencilResource>(L"Depth Buffer", DepthStencilResource::CreateResourceDesc(
        m_FrameBufferWidth,
        m_FrameBufferHeight,
        DXGI_FORMAT_D32_FLOAT
    ));

    m_CurrentBackBufferIndex = m_SwapChain->GetCurrentBackBufferIndex();
}

ETH_NAMESPACE_END

