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

#include "graphics/core.h"
#include "graphics/graphicdisplay.h"
#include "graphics/rhi/rhidevice.h"
#include "graphics/rhi/rhiswapchain.h"
#include "graphics/rhi/rhiresourceviews.h"

constexpr uint32_t DefaultFrameBufferWidth = 1920;
constexpr uint32_t DefaultFrameBufferHeight = 1080;

Ether::Graphics::GraphicDisplay::GraphicDisplay()
    : m_BufferingMode(BufferingMode::Triple)
    , m_RtvAllocator(RhiDescriptorHeapType::Rtv)
    , m_ScissorRect({ 0, 0, 99999, 99999 })
    , m_Viewport({ 0.0f, 0.0f, (float)DefaultFrameBufferWidth, (float)DefaultFrameBufferHeight, 0.0f, 1.0f })
    , m_VSyncEnabled(false)
    , m_VSyncVBlanks(1)
{
    CreateSwapChain(Core::GetConfig().m_WindowHandle);
    CreateResourcesFromSwapChain();
    CreateViewsFromSwapChain();
    ResetFences();
}

Ether::Graphics::GraphicDisplay::~GraphicDisplay()
{
    m_SwapChain.reset();

    for (uint32_t i = 0; i < GetNumBuffers(); ++i)
    {
        m_RenderTargetRtv[i].reset();
        m_RenderTargetSrv[i].reset();
    }
}

void Ether::Graphics::GraphicDisplay::Present()
{
    ETH_MARKER_EVENT("Display - Present");
    m_SwapChain->Present(m_VSyncEnabled ? m_VSyncVBlanks : 0);

    // When using the DXGI_SWAP_EFFECT_FLIP_DISCARD flip model, 
    // the order of back buffer indices are not guaranteed to be sequential
    ResetCurrentBufferIndex();
}

void Ether::Graphics::GraphicDisplay::ResizeBuffers(ethVector2u size)
{
    Core::FlushGpu();
    ResizeViewport(size);

    m_SwapChain->ResizeBuffers({ size });

    ResetCurrentBufferIndex();
    CreateResourcesFromSwapChain();
    CreateViewsFromSwapChain();
}

void Ether::Graphics::GraphicDisplay::CreateSwapChain(void* hwnd)
{
    RhiSwapChainDesc desc = {};
    desc.m_Resolution = { DefaultFrameBufferWidth, DefaultFrameBufferHeight };
    desc.m_Format = BackBufferFormat;
    desc.m_SampleDesc = { 1, 0 };
    desc.m_BufferCount = GetNumBuffers();
    desc.m_ScalingMode = RhiScalingMode::Stretch;
    desc.m_SwapEffect = RhiSwapEffect::FlipDiscard;
    desc.m_Flag = RhiSwapChainFlag::AllowTearing;
    desc.m_CommandQueue = &Core::GetCommandManager().GetGraphicsQueue();
    desc.m_SurfaceTarget = hwnd;

    m_SwapChain = Core::GetDevice().CreateSwapChain(desc);
    ResetCurrentBufferIndex();
}

void Ether::Graphics::GraphicDisplay::CreateResourcesFromSwapChain()
{
    for (uint32_t i = 0; i < GetNumBuffers(); ++i)
        m_RenderTargets[i] = &m_SwapChain->GetBuffer(i);
}

void Ether::Graphics::GraphicDisplay::CreateViewsFromSwapChain()
{
    m_DescriptorLifetimeMaintainer.clear();

    auto rtvAllocation = m_RtvAllocator.Allocate(3);
    auto srvAllocation = Core::GetGpuDescriptorAllocator().Allocate(1);

    for (uint32_t i = 0; i < GetNumBuffers(); ++i)
    {
        RhiRenderTargetViewDesc rtvDesc = {};
        rtvDesc.m_Format = BackBufferFormat;
        rtvDesc.m_Resource = m_RenderTargets[i];
        rtvDesc.m_TargetCpuHandle = dynamic_cast<DescriptorAllocation&>(*rtvAllocation).GetCpuHandle(i);

        m_RenderTargetRtv[i] = Core::GetDevice().CreateRenderTargetView(rtvDesc);

        RhiShaderResourceViewDesc srvDesc = {};
        srvDesc.m_Format = BackBufferFormat;
        srvDesc.m_Resource = m_RenderTargets[i];
        srvDesc.m_Dimensions = RhiShaderResourceDims::Texture2D;
        srvDesc.m_TargetCpuHandle = dynamic_cast<DescriptorAllocation&>(*srvAllocation).GetCpuHandle();
        srvDesc.m_TargetGpuHandle = dynamic_cast<DescriptorAllocation&>(*srvAllocation).GetGpuHandle();
        m_RenderTargetSrv[i] = Core::GetDevice().CreateShaderResourceView(srvDesc);

        Core::GetBindlessResourceManager().RegisterView(
            m_RenderTargetSrv[i]->GetViewID(),
            dynamic_cast<DescriptorAllocation&>(*srvAllocation).GetDescriptorIndex(i)
        );
    }

    m_DescriptorLifetimeMaintainer.emplace_back(std::move(rtvAllocation));
    m_DescriptorLifetimeMaintainer.emplace_back(std::move(srvAllocation));
}

void Ether::Graphics::GraphicDisplay::ResizeViewport(ethVector2u& size)
{
    m_Viewport = { 0.0f, 0.0f, (float)size.x, (float)size.y, 0.0f, 1.0f };
}

void Ether::Graphics::GraphicDisplay::ResetCurrentBufferIndex()
{
    m_CurrentBackBufferIndex = m_SwapChain->GetCurrentBackBufferIndex();
}

void Ether::Graphics::GraphicDisplay::ResetFences()
{
    for (uint32_t i = 0; i < GetNumBuffers(); ++i)
        m_FrameBufferFences[i] = 0;
}

