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

// TODO: CLEAN
#include "graphic/pipeline/graphicpipelinestate.h"
#include "graphic/resource/depthstencilresource.h"
#include "graphic/common/visual.h"
#include "graphic/common/shader.h"

ETH_NAMESPACE_BEGIN

wrl::ComPtr<ID3D12Device3> g_GraphicDevice;
CommandManager g_CommandManager;
ShaderDaemon g_ShaderDaemon;

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
    InitializeCommonStates();

    g_CommandManager.Initialize();
    g_ShaderDaemon.Initialize();
    m_Display.Initialize();
    m_Context.Initialize();
    m_BatchRenderer.Initialize();
    m_GuiManager.Initialize();
}

void GraphicManager::Shutdown()
{
    DestroyCommonStates();

    m_GuiManager.Shutdown();
    m_BatchRenderer.Shutdown();
    m_Context.Shutdown();
    m_Display.Shutdown();
    g_ShaderDaemon.Shutdown();
    g_CommandManager.Shutdown();
}

void GraphicManager::WaitForPresent()
{
    m_Context.GetCommandQueue()->StallForFence(m_Display.GetCurrentBackBufferFence());
    CleanUp();
}

void GraphicManager::Render()
{
    m_Context.TransitionResource(*m_Display.GetCurrentBackBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET);
    m_Context.ClearColor(*m_Display.GetCurrentBackBuffer(), g_EngineConfig.m_ClearColor);
    m_Context.ClearDepth(*m_Display.GetDepthBuffer(), 1.0f);

    // TODO: Move this elsewhere
    ethXMVector globalTime = DirectX::XMVectorSet(GetTimeSinceStart() / 20, GetTimeSinceStart(), GetTimeSinceStart() * 2, GetTimeSinceStart() * 3);
    m_Context.GetCommandList()->SetPipelineState(g_EngineConfig.m_RenderWireframe ? g_DefaultWireframePSO->GetPipelineStateObject() : g_DefaultPSO->GetPipelineStateObject());
    m_Context.GetCommandList()->SetGraphicsRootSignature(g_DefaultRootSignature->GetRootSignature());
    m_Context.GetCommandList()->OMSetRenderTargets(1, &m_Display.GetCurrentBackBuffer()->GetRTV(), FALSE, &m_Display.GetDepthBuffer()->GetDSV());
    m_Context.GetCommandList()->RSSetViewports(1, &m_Display.GetViewport());
    m_Context.GetCommandList()->RSSetScissorRects(1, &m_Display.GetScissorRect());
    m_Context.GetCommandList()->SetGraphicsRoot32BitConstants(0, 4, &globalTime, 0);

    for (Visual* visual : m_Visuals)
    {
        if (!visual->IsInitialized())
            visual->Initialize();

        ethXMMatrix modelMatrix = visual->GetModelMatrix();
        ethXMMatrix mvpMatrix = DirectX::XMMatrixMultiply(modelMatrix, m_Context.GetViewMatrix());
        mvpMatrix = DirectX::XMMatrixMultiply(mvpMatrix, m_Context.GetProjectionMatrix());

        m_Context.GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        m_Context.GetCommandList()->IASetVertexBuffers(0, 1, &visual->GetVertexBufferView());
        m_Context.GetCommandList()->IASetIndexBuffer(&visual->GetIndexBufferView());
        m_Context.GetCommandList()->SetGraphicsRoot32BitConstants(1, sizeof(ethXMMatrix) / 4, &mvpMatrix, 0);
        m_Context.GetCommandList()->DrawIndexedInstanced(36 /*TODO: Get this val from visual*/, 1, 0, 0, 0);
    }

    m_Context.FinalizeAndExecute();
    m_Context.Reset();
}

void GraphicManager::RenderGui()
{
    if (g_EngineConfig.IsDebugGuiEnabled())
        m_GuiManager.Render();
}

void GraphicManager::Present()
{
    m_Context.TransitionResource(*m_Display.GetCurrentBackBuffer(), D3D12_RESOURCE_STATE_PRESENT);
    m_Context.FinalizeAndExecute();
    m_Context.Reset();
    m_Display.SetCurrentBackBufferFence(m_Context.GetCommandQueue()->GetCompletionFenceValue());
    m_Display.Present();
}

void GraphicManager::RegisterVisual(Visual* visual)
{
    m_Visuals.push_back(visual);
}

void GraphicManager::DeregisterVisual(Visual* visual)
{
    for (auto iter = m_Visuals.begin(); iter != m_Visuals.end(); ++iter)
    {
        if (*iter != visual)
            continue;

        m_Visuals.erase(iter);
        return;
    }
}

void GraphicManager::CleanUp()
{
    // TODO: Clean up old PSOs and other resources
}

void GraphicManager::InitializeDebugLayer()
{
#if defined(_DEBUG)
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

