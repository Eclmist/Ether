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

#include "graphics/graphiccore.h"
#include "graphics/graphicdisplay.h"
#include "graphics/rhi/rhidevice.h"
#include "graphics/rhi/rhiswapchain.h"
#include "graphics/rhi/rhiresourceviews.h"

Ether::Graphics::GraphicDisplay::GraphicDisplay()
    : m_BufferingMode(BufferingMode::Triple)
    , m_VSyncEnabled(false)
    , m_VSyncVBlanks(1)
{
    ResizeViewport(GraphicCore::GetGraphicConfig().GetResolution());
    CreateSwapChain(GraphicCore::GetGraphicConfig().GetWindowHandle());
    CreateResourcesFromSwapChain();
    CreateViewsFromSwapChain();
    InitializeFences();
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
    UpdateBackBufferIndex();
}

void Ether::Graphics::GraphicDisplay::ResizeBuffers(const ethVector2u& size)
{
    GraphicCore::FlushGpu();
    ResizeViewport(size);

    m_SwapChain->ResizeBuffers({ size });

    UpdateBackBufferIndex();
    CreateResourcesFromSwapChain();
    CreateViewsFromSwapChain();
}

void Ether::Graphics::GraphicDisplay::InitializeFences()
{
    for (uint32_t i = 0; i < GetNumBuffers(); ++i)
        m_FrameBufferFences[i] = 0;
}

void Ether::Graphics::GraphicDisplay::ResizeViewport(const ethVector2u& size)
{
    m_Viewport = { 0.0f, 0.0f, (float)size.x, (float)size.y, 0.0f, 1.0f };
    m_ScissorRect = { 0.0f, 0.0f, (float)size.x, (float)size.y };
}

void Ether::Graphics::GraphicDisplay::CreateSwapChain(void* hwnd)
{
    RhiSwapChainDesc desc = {};
    desc.m_Resolution = { m_Viewport.m_Width, m_Viewport.m_Height };
    desc.m_Format = BackBufferFormat;
    desc.m_SampleDesc = { 1, 0 };
    desc.m_BufferCount = GetNumBuffers();
    desc.m_ScalingMode = RhiScalingMode::Stretch;
    desc.m_SwapEffect = RhiSwapEffect::FlipDiscard;
    desc.m_Flag = RhiSwapChainFlag::AllowTearing;
    desc.m_CommandQueue = &GraphicCore::GetCommandManager().GetGraphicQueue();
    desc.m_SurfaceTarget = hwnd;

    m_SwapChain = GraphicCore::GetDevice().CreateSwapChain(desc);
    UpdateBackBufferIndex();
}

void Ether::Graphics::GraphicDisplay::CreateResourcesFromSwapChain()
{
    for (uint32_t i = 0; i < GetNumBuffers(); ++i)
        m_RenderTargets[i] = &m_SwapChain->GetBuffer(i);
}

void Ether::Graphics::GraphicDisplay::CreateViewsFromSwapChain()
{
    m_SwapChainDescriptors.clear();

    auto rtvAllocation = GraphicCore::GetRtvAllocator().Allocate(3);
    auto srvAllocation = GraphicCore::GetSrvCbvUavAllocator().Allocate(1);

    for (uint32_t i = 0; i < GetNumBuffers(); ++i)
    {
        RhiRenderTargetViewDesc rtvDesc = {};
        rtvDesc.m_Format = BackBufferFormat;
        rtvDesc.m_Resource = m_RenderTargets[i];
        rtvDesc.m_TargetCpuAddr = dynamic_cast<DescriptorAllocation&>(*rtvAllocation).GetCpuAddress(i);

        m_RenderTargetRtv[i] = GraphicCore::GetDevice().CreateRenderTargetView(rtvDesc);

        RhiShaderResourceViewDesc srvDesc = {};
        srvDesc.m_Format = BackBufferFormat;
        srvDesc.m_Resource = m_RenderTargets[i];
        srvDesc.m_Dimensions = RhiShaderResourceDims::Texture2D;
        srvDesc.m_TargetCpuAddr = dynamic_cast<DescriptorAllocation&>(*srvAllocation).GetCpuAddress();
        srvDesc.m_TargetGpuAddr = dynamic_cast<DescriptorAllocation&>(*srvAllocation).GetGpuAddress();
        m_RenderTargetSrv[i] = GraphicCore::GetDevice().CreateShaderResourceView(srvDesc);

        GraphicCore::GetBindlessResourceManager().RegisterView(
            m_RenderTargetSrv[i]->GetViewID(),
            dynamic_cast<DescriptorAllocation&>(*srvAllocation).GetDescriptorIndex(i));
    }

    m_SwapChainDescriptors.emplace_back(std::move(rtvAllocation));
    m_SwapChainDescriptors.emplace_back(std::move(srvAllocation));
}

void Ether::Graphics::GraphicDisplay::UpdateBackBufferIndex()
{
    ETH_MARKER_EVENT("SwapChain - Get Current Back Buffer Index");
    m_CurrentBackBufferIndex = m_SwapChain->GetCurrentBackBufferIndex();
}
