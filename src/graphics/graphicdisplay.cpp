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
    , m_VSyncVBlanks(4)
{
    ResizeViewport(GraphicCore::GetGraphicConfig().GetResolution());
    CreateSwapChain(GraphicCore::GetGraphicConfig().GetWindowHandle());
    CreateResourcesFromSwapChain();
    CreateResourceForExport();
    CreateViewsFromSwapChain();
    InitializeFences();
}

Ether::Graphics::GraphicDisplay::~GraphicDisplay()
{
    m_SwapChain.reset();
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
    desc.m_Format = RhiFormat::R8G8B8A8Unorm; // BackBufferFormat as defined in PCH is SRGB for the sake of 
                                              // automatic gamma correction. However, swapchain must still be
                                              // created without the SRGB flag.
    desc.m_SampleDesc = { 1, 0 };
    desc.m_NumBuffers = GetNumBuffers();
    desc.m_SurfaceTarget = hwnd;

    m_SwapChain = GraphicCore::GetDevice().CreateSwapChain(desc);
    UpdateBackBufferIndex();
}

void Ether::Graphics::GraphicDisplay::CreateResourcesFromSwapChain()
{
    for (uint32_t i = 0; i < GetNumBuffers(); ++i)
        m_RenderTargets[i] = &m_SwapChain->GetBuffer(i);
}

void Ether::Graphics::GraphicDisplay::CreateResourceForExport()
{
    Graphics::CommandContext ctx("Export Readback Heap Creation", Graphics::RhiCommandType::Graphic, _16MiB);

    RhiCommitedResourceDesc desc = {};
    desc.m_ClearValue = {};
    desc.m_HeapType = RhiHeapType::Readback;
    desc.m_Name = "Export Buffer";
    desc.m_State = RhiResourceState::CopyDest;
    desc.m_ResourceDesc = RhiCreateBufferResourceDesc(m_Viewport.m_Width * m_Viewport.m_Height * 4);

    m_ExportResource = GraphicCore::GetDevice().CreateCommittedResource(desc);
}

void Ether::Graphics::GraphicDisplay::CreateViewsFromSwapChain()
{
    m_SwapChainDescriptors.clear();

    auto rtvAllocation = GraphicCore::GetRtvAllocator().Allocate(3);
    auto srvAllocation = GraphicCore::GetSrvCbvUavAllocator().Allocate(3);

    for (uint32_t i = 0; i < GetNumBuffers(); ++i)
    {
        m_RenderTargetRtv[i] = {};
        m_RenderTargetRtv[i].SetWidth(m_Viewport.m_Width);
        m_RenderTargetRtv[i].SetHeight(m_Viewport.m_Height);
        m_RenderTargetRtv[i].SetDepth(1);
        m_RenderTargetRtv[i].SetDimension(RhiResourceDimension::Texture2D);
        m_RenderTargetRtv[i].SetFormat(BackBufferLdrFormat);
        m_RenderTargetRtv[i].SetViewID("BackBuffer RTV " + std::to_string(i));
        m_RenderTargetRtv[i].SetResourceID(m_RenderTargets[i]->GetResourceID());
        m_RenderTargetRtv[i].SetCpuAddress(((DescriptorAllocation&)(*rtvAllocation)).GetCpuAddress(i));
        GraphicCore::GetDevice().InitializeRenderTargetView(m_RenderTargetRtv[i], *m_RenderTargets[i]);

        m_RenderTargetSrv[i] = {};
        m_RenderTargetRtv[i].SetWidth(m_Viewport.m_Width);
        m_RenderTargetRtv[i].SetHeight(m_Viewport.m_Height);
        m_RenderTargetRtv[i].SetDepth(1);
        m_RenderTargetRtv[i].SetDimension(RhiResourceDimension::Texture2D);
        m_RenderTargetSrv[i].SetFormat(BackBufferLdrFormat);
        m_RenderTargetRtv[i].SetViewID("BackBuffer SRV " + std::to_string(i));
        m_RenderTargetSrv[i].SetResourceID(m_RenderTargets[i]->GetResourceID());
        m_RenderTargetSrv[i].SetDimension(RhiResourceDimension::Texture2D);
        m_RenderTargetSrv[i].SetCpuAddress(((DescriptorAllocation&)(*srvAllocation)).GetCpuAddress(i));
        m_RenderTargetSrv[i].SetGpuAddress(((DescriptorAllocation&)(*srvAllocation)).GetGpuAddress(i));
        GraphicCore::GetDevice().InitializeShaderResourceView(m_RenderTargetSrv[i], *m_RenderTargets[i]);
    }

    m_SwapChainDescriptors.emplace_back(std::move(rtvAllocation));
    m_SwapChainDescriptors.emplace_back(std::move(srvAllocation));
}

void Ether::Graphics::GraphicDisplay::UpdateBackBufferIndex()
{
    ETH_MARKER_EVENT("SwapChain - Get Current Back Buffer Index");
    m_CurrentBackBufferIndex = m_SwapChain->GetCurrentBackBufferIndex();
}

