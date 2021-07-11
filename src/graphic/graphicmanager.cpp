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
#include "core/entity/component/transformcomponent.h"

ETH_NAMESPACE_BEGIN

wrl::ComPtr<ID3D12Device3> g_GraphicDevice;
CommandManager g_CommandManager;
ShaderDaemon g_ShaderDaemon;

void LoadEngineContent();

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
    m_GuiManager.Initialize();

    LoadEngineContent(); // TODO: Remove
}

void GraphicManager::Shutdown()
{
    m_GuiManager.Shutdown();
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

std::shared_ptr<GraphicPipelineState> pso;
std::shared_ptr<GraphicPipelineState> wireframePso;
wrl::ComPtr<ID3D12DescriptorHeap> g_DSVHeap;
extern DXGI_FORMAT g_SwapChainFormat;
RootSignature tempRS(2);
D3D12_INPUT_ELEMENT_DESC tempInputLayout[] =
{
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    { "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
};

std::shared_ptr<DepthStencilResource> m_DepthBuffer;
Visual debugVisual;
TransformComponent* debugTransform;

Shader vertexShader(L"default_vs.hlsl", L"VS_Main", L"vs_6_0", ShaderType::SHADERTYPE_VS);
Shader pixelShader(L"default_ps.hlsl", L"PS_Main", L"ps_6_0", ShaderType::SHADERTYPE_PS);

void LoadEngineContent()
{
    vertexShader.Compile();
    pixelShader.Compile();

    m_DepthBuffer = std::make_shared<DepthStencilResource>(L"Depth Buffer", DepthStencilResource::CreateResourceDesc(
        g_EngineConfig.GetClientWidth(),
        g_EngineConfig.GetClientHeight(),
        DXGI_FORMAT_D32_FLOAT
    ));

    D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

    tempRS[0].SetAsConstant(4, 0, D3D12_SHADER_VISIBILITY_ALL);
    tempRS[1].SetAsConstant(16, 1, D3D12_SHADER_VISIBILITY_ALL);
    tempRS.Finalize(L"tempRS", rootSignatureFlags);

    pso = std::make_shared<GraphicPipelineState>();
    pso->SetBlendState(g_BlendDisabled);
    pso->SetRasterizerState(g_RasterizerDefault);
    pso->SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
    pso->SetVertexShader(vertexShader.GetCompiledShader(), vertexShader.GetCompiledShaderSize());
    pso->SetPixelShader(pixelShader.GetCompiledShader(), pixelShader.GetCompiledShaderSize());
    pso->SetRenderTargetFormat(g_SwapChainFormat);
    pso->SetDepthTargetFormat(DXGI_FORMAT_D32_FLOAT);
    pso->SetDepthStencilState(g_DepthStateReadWrite);
    pso->SetSamplingDesc(1, 0);
    // Shader must contain input layout
    pso->SetNumLayoutElements(2);
    pso->SetInputLayout(tempInputLayout);
    pso->SetRootSignature(tempRS);
    pso->SetSampleMask(0xFFFFFFFF);
    pso->Finalize();

    wireframePso = std::make_shared<GraphicPipelineState>();
    wireframePso->SetBlendState(g_BlendDisabled);
    wireframePso->SetRasterizerState(g_RasterizerWireframe);
    wireframePso->SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
    // TODO: perhaps pass in IDXCBlob directly?
    wireframePso->SetVertexShader(vertexShader.GetCompiledShader(), vertexShader.GetCompiledShaderSize());
    wireframePso->SetPixelShader(pixelShader.GetCompiledShader(), pixelShader.GetCompiledShaderSize());
    wireframePso->SetRenderTargetFormat(g_SwapChainFormat);
    wireframePso->SetDepthTargetFormat(DXGI_FORMAT_D32_FLOAT);
    wireframePso->SetDepthStencilState(g_DepthStateReadWrite);
    wireframePso->SetSamplingDesc(1, 0);
    // Shader must contain input layout
    wireframePso->SetNumLayoutElements(2);
    wireframePso->SetInputLayout(tempInputLayout);
    wireframePso->SetRootSignature(tempRS);
    wireframePso->SetSampleMask(0xFFFFFFFF);
    wireframePso->Finalize();

    auto entities = g_World.GetEntities();
    MeshComponent* mesh = dynamic_cast<MeshComponent*>(entities[0]->GetAllComponents()[1]);
    debugVisual.Initialize(*mesh);
    debugTransform = dynamic_cast<TransformComponent*>(entities[0]->GetAllComponents()[0]);
}

void GraphicManager::Render()
{
    m_Context.TransitionResource(*m_Display.GetCurrentBackBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET);
    m_Context.ClearColor(*m_Display.GetCurrentBackBuffer(), g_EngineConfig.m_ClearColor);

    /*
     TEMP RENDERING CODE FOR DRAWING GEOMETRY =============================================
    */
    D3D12_RECT m_ScissorRect = (CD3DX12_RECT(0, 0, LONG_MAX, LONG_MAX));
    D3D12_VIEWPORT m_Viewport = (CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(g_EngineConfig.GetClientWidth()), static_cast<float>(g_EngineConfig.GetClientHeight())));

    ethXMVector time = DirectX::XMVectorSet(GetTimeSinceStart() / 20, GetTimeSinceStart(), GetTimeSinceStart() * 2, GetTimeSinceStart() * 3);
    ethXMMatrix modelMatrix = debugTransform->GetMatrix();
    ethXMMatrix mvpMatrix = DirectX::XMMatrixMultiply(modelMatrix, m_Context.GetViewMatrix());
    mvpMatrix = DirectX::XMMatrixMultiply(mvpMatrix, m_Context.GetProjectionMatrix());

    // Shader recompile
    if (vertexShader.HasRecompiled())
    {
        pso->SetVertexShader(vertexShader.GetCompiledShader(), vertexShader.GetCompiledShaderSize());
        pso->Finalize();
        wireframePso->SetVertexShader(vertexShader.GetCompiledShader(), vertexShader.GetCompiledShaderSize());
        wireframePso->Finalize();
        vertexShader.SetRecompiled(false);
    }
    if (pixelShader.HasRecompiled())
    {
        pso->SetPixelShader(pixelShader.GetCompiledShader(), pixelShader.GetCompiledShaderSize());
        pso->Finalize();
        wireframePso->SetPixelShader(pixelShader.GetCompiledShader(), pixelShader.GetCompiledShaderSize());
        wireframePso->Finalize();
        pixelShader.SetRecompiled(false);
    }

    m_Context.GetCommandList()->ClearDepthStencilView(m_DepthBuffer->GetDSV(), D3D12_CLEAR_FLAG_DEPTH, 1.0, 0, 0, nullptr);
    m_Context.GetCommandList()->SetPipelineState(g_EngineConfig.m_RenderWireframe ? wireframePso->GetPipelineStateObject() : pso->GetPipelineStateObject());
    m_Context.GetCommandList()->SetGraphicsRootSignature(tempRS.GetRootSignature());
    m_Context.GetCommandList()->OMSetRenderTargets(1, &m_Display.GetCurrentBackBuffer()->GetRTV(), FALSE, &m_DepthBuffer->GetDSV());
    m_Context.GetCommandList()->RSSetViewports(1, &m_Viewport);
    m_Context.GetCommandList()->RSSetScissorRects(1, &m_ScissorRect);
    m_Context.GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_Context.GetCommandList()->IASetVertexBuffers(0, 1, &debugVisual.GetVertexBufferView());
    m_Context.GetCommandList()->IASetIndexBuffer(&debugVisual.GetIndexBufferView());
    m_Context.GetCommandList()->SetGraphicsRoot32BitConstants(0, 4, &time, 0);
    m_Context.GetCommandList()->SetGraphicsRoot32BitConstants(1, sizeof(ethXMMatrix) / 4, &mvpMatrix, 0);
    m_Context.GetCommandList()->DrawIndexedInstanced(36, 1001, 0, 0, 0);
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

