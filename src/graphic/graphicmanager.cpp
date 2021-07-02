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

#include "graphicmanager.h"

ETH_NAMESPACE_BEGIN

wrl::ComPtr<ID3D12Device3> g_GraphicDevice;
CommandManager g_CommandManager;

/*
Enable the debug layer
Create the device
Create the command queue - CommandManager::CommandQueue
Create the swap chain - Display
Create a render target view (RTV) descriptor heap - Display::TextureResource
Create frame resources (a render target view for each frame) - Display::TextureResource
Create a command allocator - CommandManager::CommandQueue::CommandAllocatorPool
*/
void GraphicManager::Initialize()
{
    if (g_GraphicDevice != nullptr)
    {
        LogGraphicsWarning("An attempt was made to initialize an already initialized Renderer");
        return;
    }

    LogGraphicsInfo("Initializing Renderer");

    InitializeDebugLayer();
    InitializeAdapter();
    InitializeDevice();

    g_CommandManager.Initialize();
    m_Display.Initialize();
    m_Context.Initialize();
    m_GuiManager.Initialize();
}

void GraphicManager::Shutdown()
{
    m_GuiManager.Shutdown();
    m_Context.Shutdown();
    m_Display.Shutdown();
    g_CommandManager.Shutdown();
}

void GraphicManager::WaitForPresent()
{
    m_Context.GetCommandQueue()->StallForFence(m_Display.GetCurrentBackBufferFence());
}

void GraphicManager::Render()
{
    m_Context.TransitionResource(*m_Display.GetCurrentBackBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET);
    m_Context.ClearColor(*m_Display.GetCurrentBackBuffer(), Ether::ethVector4(0.05, 0.0, 0.07, 0.0));
    m_Context.FinalizeAndExecute();
    m_Context.Reset();
}

void GraphicManager::RenderGui()
{
    if (g_EngineConfig.IsDebugModeEnabled())
        m_GuiManager.Render();
}

void GraphicManager::Present()
{
    m_Context.TransitionResource(*m_Display.GetCurrentBackBuffer(), D3D12_RESOURCE_STATE_PRESENT);
    m_Context.FinalizeAndExecute();
    m_Context.Reset();
    m_Display.SetCurrentBackBufferFence(m_Context.GetCommandQueue()->GetCompletionFence());
    m_Display.Present();
}

void GraphicManager::InitializeDebugLayer()
{
#if defined(_DEBUG)
    // Always enable the debug layer before doing anything DX12 related
    // so all possible errors generated while creating DX12 objects
    // are caught by the debug layer.
    wrl::ComPtr<ID3D12Debug> debugInterface;
    ASSERT_SUCCESS(D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface)));
    debugInterface->EnableDebugLayer();
#endif
}

void GraphicManager::InitializeAdapter()
{
    wrl::ComPtr<IDXGIAdapter1> dxgiAdapter1;

    wrl::ComPtr<IDXGIFactory4> dxgiFactory;
    ASSERT_SUCCESS(CreateDXGIFactory2(0, IID_PPV_ARGS(&dxgiFactory)));

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
        ASSERT_SUCCESS(dxgiAdapter1.As(&m_Adapter));
    }
}

void GraphicManager::InitializeDevice()
{
    ASSERT_SUCCESS(D3D12CreateDevice(m_Adapter.Get(), ETH_MINIMUM_FEATURE_LEVEL, IID_PPV_ARGS(&g_GraphicDevice)));
}

ETH_NAMESPACE_END
