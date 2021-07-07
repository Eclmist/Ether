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

    // Add to graphiccommon.h (TODO)
    // along with a bunch of common depth stencil states. 
    // we can reuse these descs in lots of places
    D3D12_DEPTH_STENCIL_DESC depthStateReadWrite = {};
    depthStateReadWrite.StencilEnable = false;
    depthStateReadWrite.DepthEnable = true;
    depthStateReadWrite.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    depthStateReadWrite.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

    D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

    // Add to graphiccommon.h (TODO)
    D3D12_BLEND_DESC alphaBlend = {};
    alphaBlend.IndependentBlendEnable = FALSE;
    alphaBlend.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
    alphaBlend.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
    alphaBlend.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
    alphaBlend.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
    alphaBlend.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
    alphaBlend.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
    alphaBlend.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    alphaBlend.RenderTarget[0].BlendEnable = FALSE;

    D3D12_RASTERIZER_DESC RasterizerDefault = {};
    RasterizerDefault.FillMode = D3D12_FILL_MODE_SOLID;
    RasterizerDefault.CullMode = D3D12_CULL_MODE_BACK;
    RasterizerDefault.FrontCounterClockwise = FALSE;
    RasterizerDefault.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
    RasterizerDefault.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
    RasterizerDefault.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
    RasterizerDefault.DepthClipEnable = TRUE;
    RasterizerDefault.MultisampleEnable = FALSE;
    RasterizerDefault.AntialiasedLineEnable = FALSE;
    RasterizerDefault.ForcedSampleCount = 0;
    RasterizerDefault.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

    tempRS[0].SetAsConstant(4, 0, D3D12_SHADER_VISIBILITY_ALL);
    tempRS[1].SetAsConstant(16, 1, D3D12_SHADER_VISIBILITY_ALL);
    tempRS.Finalize(L"tempRS", rootSignatureFlags);

    pso = std::make_shared<GraphicPipelineState>();
    pso->SetBlendState(alphaBlend);
    pso->SetRasterizerState(RasterizerDefault);
    pso->SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
    // TODO: perhaps pass in IDXCBlob directly?
    pso->SetVertexShader(vertexShader.GetCompiledShaderBlob()->GetBufferPointer(), vertexShader.GetCompiledShaderBlob()->GetBufferSize());
    pso->SetPixelShader(pixelShader.GetCompiledShaderBlob()->GetBufferPointer(), pixelShader.GetCompiledShaderBlob()->GetBufferSize());
    pso->SetRenderTargetFormat(g_SwapChainFormat);
    pso->SetDepthTargetFormat(DXGI_FORMAT_D32_FLOAT);
    pso->SetDepthStencilState(depthStateReadWrite);
    pso->SetSamplingDesc(1, 0);
    // Shader must contain input layout
    pso->SetNumLayoutElements(2);
    pso->SetInputLayout(tempInputLayout);
    pso->SetRootSignature(tempRS);
    pso->SetSampleMask(0xFFFFFFFF);
    pso->Finalize();


    RasterizerDefault.FillMode = D3D12_FILL_MODE_WIREFRAME;

    wireframePso = std::make_shared<GraphicPipelineState>();
    wireframePso->SetBlendState(alphaBlend);
    wireframePso->SetRasterizerState(RasterizerDefault);
    wireframePso->SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
    // TODO: perhaps pass in IDXCBlob directly?
    wireframePso->SetVertexShader(vertexShader.GetCompiledShaderBlob()->GetBufferPointer(), vertexShader.GetCompiledShaderBlob()->GetBufferSize());
    wireframePso->SetPixelShader(pixelShader.GetCompiledShaderBlob()->GetBufferPointer(), pixelShader.GetCompiledShaderBlob()->GetBufferSize());
    wireframePso->SetRenderTargetFormat(g_SwapChainFormat);
    wireframePso->SetDepthTargetFormat(DXGI_FORMAT_D32_FLOAT);
    wireframePso->SetDepthStencilState(depthStateReadWrite);
    wireframePso->SetSamplingDesc(1, 0);
    // Shader must contain input layout
    wireframePso->SetNumLayoutElements(2);
    wireframePso->SetInputLayout(tempInputLayout);
    wireframePso->SetRootSignature(tempRS);
    wireframePso->SetSampleMask(0xFFFFFFFF);
    wireframePso->Finalize();

    auto entities = g_World.GetEntities();
    MeshComponent* mesh = nullptr;

    for (auto e : entities)
        if (e->GetAllComponents().size() > 0)
        {
            mesh = dynamic_cast<MeshComponent*>(e->GetAllComponents()[0]);
            break;
        }

    debugVisual.Initialize(*mesh);
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

    // Shader recompile
    if (vertexShader.HasRecompiled())
    {
        pso->SetVertexShader(vertexShader.GetCompiledShaderBlob()->GetBufferPointer(), vertexShader.GetCompiledShaderBlob()->GetBufferSize());
        pso->Finalize();

        wireframePso->SetVertexShader(vertexShader.GetCompiledShaderBlob()->GetBufferPointer(), vertexShader.GetCompiledShaderBlob()->GetBufferSize());
        wireframePso->Finalize();
        vertexShader.SetRecompiled(false);
    }

    if (pixelShader.HasRecompiled())
    {
        pso->SetPixelShader(pixelShader.GetCompiledShaderBlob()->GetBufferPointer(), pixelShader.GetCompiledShaderBlob()->GetBufferSize());
        pso->Finalize();

        wireframePso->SetPixelShader(pixelShader.GetCompiledShaderBlob()->GetBufferPointer(), pixelShader.GetCompiledShaderBlob()->GetBufferSize());
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

    double timeSinceStart = GetTimeSinceStart();

    // Update the model matrix.
    float angle = static_cast<float>(timeSinceStart);
    const ethXMVector rotationAxis = DirectX::XMVectorSet(0, 1, 1, 0);
    ethXMMatrix modelMatrix = DirectX::XMMatrixRotationAxis(rotationAxis, DirectX::XMConvertToRadians(angle));
    const ethXMVector eyePosition = DirectX::XMVectorSet(0, 0, -10, 1);
    const ethXMVector focusPoint = DirectX::XMVectorSet(0, 0, 0, 1);
    const ethXMVector upDirection = DirectX::XMVectorSet(0, 1, 0, 0);
    ethXMMatrix viewMatrix = DirectX::XMMatrixLookAtLH(eyePosition, focusPoint, upDirection);

    float aspectRatio = g_EngineConfig.GetClientWidth() / static_cast<float>(g_EngineConfig.GetClientHeight());
    ethXMMatrix projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(80), aspectRatio, 0.1f, 100.0f);
    ethXMMatrix mvpMatrix = DirectX::XMMatrixMultiply(modelMatrix, viewMatrix);
    mvpMatrix = DirectX::XMMatrixMultiply(mvpMatrix, projectionMatrix);


    ethXMVector time = DirectX::XMVectorSet(timeSinceStart / 20, timeSinceStart, timeSinceStart * 2, timeSinceStart * 3);
    m_Context.GetCommandList()->SetGraphicsRoot32BitConstants(0, 4, &time, 0);
    m_Context.GetCommandList()->SetGraphicsRoot32BitConstants(1, sizeof(ethXMMatrix) / 4, &mvpMatrix, 0);
    m_Context.GetCommandList()->DrawIndexedInstanced(36, 1000, 0, 0, 0);

    /*
    =======================================================================================
    */

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

