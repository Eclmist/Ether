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
#include "graphic/rhi/rhidevice.h"
#include "graphic/rhi/rhiswapchain.h"

ETH_NAMESPACE_BEGIN

GraphicDisplay::GraphicDisplay()
    : m_FrameBufferWidth(EngineCore::GetEngineConfig().GetClientWidth())
    , m_FrameBufferHeight(EngineCore::GetEngineConfig().GetClientHeight())
    , m_BufferingMode(BufferingMode::Triple)
    , m_ScissorRect({ 0, 0, 99999, 99999 })
    , m_Viewport({ 0.0f, 0.0f, (float)m_FrameBufferWidth, (float)m_FrameBufferHeight, 0.0f, 1.0f })
    , m_VSyncEnabled(false)
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
    }
}

void GraphicDisplay::Present()
{
    m_SwapChain->Present(m_VSyncEnabled ? m_VSyncVBlanks : 0);

    // When using the DXGI_SWAP_EFFECT_FLIP_DISCARD flip model, 
    // the order of back buffer indices are not guaranteed to be sequential
    ResetCurrentBufferIndex();
}

void GraphicDisplay::Resize(uint32_t width, uint32_t height)
{
    if (m_FrameBufferWidth == width && m_FrameBufferHeight == height)
        return;

    m_FrameBufferWidth = width;
    m_FrameBufferHeight = height;
    m_Viewport = { 0.0f, 0.0f, (float)m_FrameBufferWidth, (float)m_FrameBufferHeight, 0.0f, 1.0f };

    GraphicCore::FlushGpu();

    for (uint32_t i = 0; i < GetNumBuffers(); ++i)
        m_RenderTargets[i].Destroy();

    RHIResizeDesc resizeDesc = {};
    resizeDesc.m_Width = m_FrameBufferWidth;
    resizeDesc.m_Height = m_FrameBufferHeight;
    ASSERT_SUCCESS(m_SwapChain->ResizeBuffers(resizeDesc));
    ResetCurrentBufferIndex();
    CreateResourcesFromSwapChain();
    CreateViewsFromSwapChain();
}

RHIResourceHandle GraphicDisplay::GetCurrentBackBuffer() const
{
    return m_RenderTargets[m_CurrentBackBufferIndex];
}

RHIRenderTargetViewHandle GraphicDisplay::GetCurrentBackBufferRTV() const
{
    return m_RenderTargetViews[m_CurrentBackBufferIndex];
}

void GraphicDisplay::CreateSwapChain()
{
    RHISwapChainDesc desc = {};
    desc.m_Width = m_FrameBufferWidth;
    desc.m_Height = m_FrameBufferHeight;
    desc.m_Format = BackBufferFormat;
    desc.m_SampleDesc = { 1, 0 };
    desc.m_BufferCount = GetNumBuffers();
    desc.m_ScalingMode = RHIScalingMode::Stretch;
    desc.m_SwapEffect = RHISwapEffect::FlipDiscard;
    desc.m_Flag = RHISwapChainFlag::AllowTearing;
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
	for (uint32_t i = 0; i < GetNumBuffers(); ++i)
	{
        RHIRenderTargetViewDesc desc = {};
        desc.m_Format = BackBufferFormat;
        desc.m_Resource = m_RenderTargets[i];
        GraphicCore::GetDevice()->CreateRenderTargetView(desc, m_RenderTargetViews[i]);
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

