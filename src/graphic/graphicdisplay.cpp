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

#include "graphicdisplay.h"
#include "graphic/rhi/rhidevice.h"
#include "graphic/rhi/rhiswapchain.h"

ETH_NAMESPACE_BEGIN

GraphicDisplay::GraphicDisplay()
    : m_FrameBufferWidth(EngineCore::GetEngineConfig().GetClientWidth())
    , m_FrameBufferHeight(EngineCore::GetEngineConfig().GetClientHeight())
    , m_BufferingMode(BufferingMode::Triple)
    , m_ScissorRect({ 0, 0, 99999, 99999 })
    , m_Viewport({ 0.0f, 0.0f, (float)m_FrameBufferWidth, (float)m_FrameBufferHeight, 0.0f, 1.0f })
    , m_VSyncEnabled(true)
    , m_VSyncVBlanks(1)
{
    CreateSwapChain();
    CreateResourcesFromSwapChain();
    CreateViewsFromSwapChain();
    ResetFences();
}

GraphicDisplay::~GraphicDisplay()
{
    m_SwapChain.Destroy();

    for (uint32_t i = 0; i < GetNumBuffers(); ++i)
    {
        m_RenderTargets[i].Destroy();
        m_RenderTargetViews[i].Destroy();
        m_ShaderResourceViews[i].Destroy();
    }
}

void GraphicDisplay::Present()
{
    m_SwapChain->Present(m_VSyncEnabled ? m_VSyncVBlanks : 0);

    // When using the DXGI_SWAP_EFFECT_FLIP_DISCARD flip model, 
    // the order of back buffer indices are not guaranteed to be sequential
    ResetCurrentBufferIndex();
}

void GraphicDisplay::ResizeBuffers()
{
    std::lock_guard<std::mutex> guard(m_WindowsResizeMutex);

    if (!m_ShouldResize)
        return;

    m_ShouldResize = false;
    m_Viewport = { 0.0f, 0.0f, (float)m_FrameBufferWidth, (float)m_FrameBufferHeight, 0.0f, 1.0f };

    GraphicCore::FlushGpu();

    for (uint32_t i = 0; i < GetNumBuffers(); ++i)
        m_RenderTargets[i].Destroy();

    RhiResizeDesc resizeDesc = {};
    resizeDesc.m_Width = m_FrameBufferWidth;
    resizeDesc.m_Height = m_FrameBufferHeight;
    ASSERT_SUCCESS(m_SwapChain->ResizeBuffers(resizeDesc));
    ResetCurrentBufferIndex();
    CreateResourcesFromSwapChain();
    CreateViewsFromSwapChain();
}

RhiResourceHandle GraphicDisplay::GetCurrentBackBuffer() const
{
    return m_RenderTargets[m_CurrentBackBufferIndex];
}

RhiRenderTargetViewHandle GraphicDisplay::GetCurrentBackBufferRTV() const
{
    return m_RenderTargetViews[m_CurrentBackBufferIndex];
}

RhiShaderResourceViewHandle GraphicDisplay::GetCurrentBackBufferSRV() const
{
    return m_ShaderResourceViews[m_CurrentBackBufferIndex];
}

void GraphicDisplay::QueueBufferResize(uint32_t width, uint32_t height)
{
    std::lock_guard<std::mutex> guard(m_WindowsResizeMutex);
    m_ShouldResize = true;
    m_FrameBufferWidth = width;
    m_FrameBufferHeight = height;
}

void GraphicDisplay::CreateSwapChain()
{
    RhiSwapChainDesc desc = {};
    desc.m_Width = m_FrameBufferWidth;
    desc.m_Height = m_FrameBufferHeight;
    desc.m_Format = BackBufferFormat;
    desc.m_SampleDesc = { 1, 0 };
    desc.m_BufferCount = GetNumBuffers();
    desc.m_ScalingMode = RhiScalingMode::Stretch;
    desc.m_SwapEffect = RhiSwapEffect::FlipDiscard;
    desc.m_Flag = RhiSwapChainFlag::AllowTearing;
    desc.m_CommandQueue = GraphicCore::GetCommandManager().GetGraphicsQueue();
    desc.m_WindowHandle = EngineCore::GetMainWindow().GetWindowHandle();

    GraphicCore::GetDevice()->CreateSwapChain(desc, m_SwapChain);
    ResetCurrentBufferIndex();
}

void GraphicDisplay::CreateResourcesFromSwapChain()
{
    for (uint32_t i = 0; i < GetNumBuffers(); ++i)
    {
        m_RenderTargets[i].Destroy();
        m_RenderTargets[i] = m_SwapChain->GetBuffer(i);
    }
}

void GraphicDisplay::CreateViewsFromSwapChain()
{
    static auto m_RtvDescriptors = GraphicCore::GetGlobalRtvDescriptorAllocator().Allocate(3);

    for (uint32_t i = 0; i < GetNumBuffers(); ++i)
    {
        RhiRenderTargetViewDesc desc = {};
        desc.m_Format = BackBufferFormat;
        desc.m_Resource = m_RenderTargets[i];
        desc.m_CpuHandle = m_RtvDescriptors->GetDescriptorHandle(i);
        GraphicCore::GetDevice()->CreateRenderTargetView(desc, m_RenderTargetViews[i]);

        RhiShaderResourceViewDesc srvDesc = {};
        srvDesc.m_Dimensions = RhiShaderResourceDims::Texture2D;
        srvDesc.m_Format = BackBufferFormat;
        srvDesc.m_Resource = m_RenderTargets[i];
        GraphicCore::GetDevice()->CreateShaderResourceView(srvDesc, m_ShaderResourceViews[i]);
    }
}

void GraphicDisplay::ResetCurrentBufferIndex()
{
    m_CurrentBackBufferIndex = m_SwapChain->GetCurrentBackBufferIndex();
}

void GraphicDisplay::ResetFences()
{
    for (uint32_t i = 0; i < GetNumBuffers(); ++i)
        m_FrameBufferFences[i] = 0;
}

ETH_NAMESPACE_END

