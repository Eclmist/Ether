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

ETH_NAMESPACE_BEGIN

void GraphicCore::Initialize()
{
    if (Instance().m_GraphicDevice != nullptr)
    {
        LogGraphicsWarning("An attempt was made to initialize an already initialized Renderer");
        return;
    }

    Instance().InitializeDebugLayer();
    Instance().InitializeAdapter();
    Instance().InitializeDevice();

    Instance().m_ShaderDaemon = std::make_unique<ShaderDaemon>();
    Instance().m_CommandManager = std::make_unique<CommandManager>();
    Instance().m_GraphicDisplay = std::make_unique<GraphicDisplay>();
    Instance().m_GraphicRenderer = std::make_unique<GraphicRenderer>();
    Instance().m_GuiRenderer = std::make_unique<GuiRenderer>();
    Instance().m_GraphicCommon = std::make_unique<GraphicCommon>();
    Instance().m_IsInitialized = true;
}

void GraphicCore::Render()
{
    Instance().m_GraphicRenderer->WaitForPresent();
    Instance().m_GraphicRenderer->Render();
    Instance().m_GuiRenderer->Render();
    Instance().m_GraphicRenderer->Present();
}

void GraphicCore::Shutdown()
{
    FlushGpu();
    Instance().m_GraphicCommon.reset();
    Instance().m_GuiRenderer.reset();
    Instance().m_GraphicRenderer.reset();
    Instance().m_GraphicDisplay.reset();
    Instance().m_CommandManager.reset();
    Instance().m_ShaderDaemon.reset();
    Reset();
}

bool GraphicCore::IsInitialized()
{
    return HasInstance() && Instance().m_IsInitialized;
}

void GraphicCore::FlushGpu()
{
    GetCommandManager().Flush();
}

void GraphicCore::InitializeDebugLayer()
{
#if defined(_DEBUG)
    wrl::ComPtr<ID3D12Debug> debugInterface;
    ASSERT_SUCCESS(D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface)));
    debugInterface->EnableDebugLayer();
#endif
}

void GraphicCore::InitializeAdapter()
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

void GraphicCore::InitializeDevice()
{
    ASSERT_SUCCESS(D3D12CreateDevice(m_Adapter.Get(), ETH_MINIMUM_FEATURE_LEVEL, IID_PPV_ARGS(&m_GraphicDevice)));
}

ETH_NAMESPACE_END

