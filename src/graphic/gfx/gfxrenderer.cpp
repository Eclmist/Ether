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

#pragma once

#include "gfxrenderer.h"
#include "core/engine.h"
#include "system/win32/window.h"

GfxRenderer::GfxRenderer()
    : EngineSubsystem()
{
}

void GfxRenderer::Initialize()
{
    InitRendererCore();
    InitCommandQueues();
    InitRTVDescriptor();
    InitSRVDescriptor();
    InitSwapChain();
    InitFences();
    InitUtilityCommandList();
    InitGuiManager();

    SetInitialized(true);
}

void GfxRenderer::InitRendererCore()
{
    EnableDebugLayer();
    m_Adapter = std::make_unique<DX12Adapter>();
    m_Device = std::make_unique<DX12Device>(m_Adapter->Get());
    m_Context = std::make_unique<GfxContext>(*m_Device, *this);

    m_View = std::make_unique<GfxView>();
    m_View->UpdateView(Engine::GetInstance().GetEngineConfig().GetClientWidth(), Engine::GetInstance().GetEngineConfig().GetClientHeight());
}

void GfxRenderer::Shutdown()
{
    Flush();
}

void GfxRenderer::Flush()
{
    for (int i = 0; i < ETH_NUM_SWAPCHAIN_BUFFERS; ++i)
    {
        m_FenceValues[i]++;
        m_DirectCommandQueue->Signal(*m_Fence, m_FenceValues[i]);
        m_Fence->WaitForFenceValue(m_FenceValues[i]);
    }
}

void GfxRenderer::RenderFrame()
{
    assert(IsInitialized() && "Trying to render frame before initialization");

    ResetUtilityCommandLists();
    ClearRenderTarget();
    RenderGui();
    Present();
    EndOfFrame();
}

void GfxRenderer::ToggleGui()
{
    m_ImGui.ToggleVisible();
}

void GfxRenderer::Resize(uint32_t width, uint32_t height)
{
    // Flush the graphics device to make sure that the swap chain buffers that we're
    // about to release are not referenced by any in-flight command lists
    Flush();

    m_View->UpdateView(width, height);

    // Buffers are released
    m_SwapChain->ResizeBuffers(width, height);

    // Recreate buffers with new size
    m_SwapChain->CreateRenderTargetViews(m_Device->Get(), m_RTVDescriptorHeap->Get());
}

void GfxRenderer::SetRTCommonParams(DX12CommandList& commandList) const
{
    // Setup the rasterizer states
    commandList.Get()->RSSetViewports(1, &m_View->GetViewport());
    commandList.Get()->RSSetScissorRects(1, &m_View->GetScissorRect());

    // Bind render target to the output merger
    auto rtv = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_RTVDescriptorHeap->Get()->GetCPUDescriptorHandleForHeapStart(),
        m_SwapChain->GetCurrentBackBufferIndex(), m_RTVDescriptorSize);
    commandList.Get()->OMSetRenderTargets(1, &rtv, false, nullptr);
}

void GfxRenderer::AddProducer(GfxProducer& producer)
{
    m_Producers.push_back(&producer);
}

void GfxRenderer::InitSwapChain()
{
    m_SwapChain = std::make_unique<DX12SwapChain>(
        EtherGame::g_hWnd,
        m_Device->Get(),
        m_DirectCommandQueue->Get(),
        Engine::GetInstance().GetEngineConfig().GetClientWidth(),
        Engine::GetInstance().GetEngineConfig().GetClientHeight());

    m_SwapChain->CreateRenderTargetViews(m_Device->Get(), m_RTVDescriptorHeap->Get());
}

void GfxRenderer::InitCommandQueues()
{
    m_DirectCommandQueue = std::make_unique<DX12CommandQueue>(m_Device->Get(), D3D12_COMMAND_LIST_TYPE_DIRECT);
    m_ComputeCommandQueue = std::make_unique<DX12CommandQueue>(m_Device->Get(), D3D12_COMMAND_LIST_TYPE_COMPUTE);
    m_CopyCommandQueue = std::make_unique<DX12CommandQueue>(m_Device->Get(), D3D12_COMMAND_LIST_TYPE_COPY);
}

void GfxRenderer::InitRTVDescriptor()
{
    m_RTVDescriptorHeap = std::make_unique<DX12DescriptorHeap>(
        m_Device->Get(),
        D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
        ETH_NUM_SWAPCHAIN_BUFFERS
        );

    m_RTVDescriptorSize = m_Device->Get()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
}

void GfxRenderer::InitSRVDescriptor()
{
    m_SRVDescriptorHeap = std::make_unique<DX12DescriptorHeap>(
        m_Device->Get(),
        D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
        1,
        D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE
        );
}

void GfxRenderer::InitFences()
{
    m_Fence = std::make_unique<DX12Fence>(m_Device->Get());

    for (int i = 0; i < ETH_NUM_SWAPCHAIN_BUFFERS; ++i)
        m_FenceValues[i] = 0;
}

void GfxRenderer::InitUtilityCommandList()
{
    for (int i = 0; i < ETH_NUM_SWAPCHAIN_BUFFERS; ++i)
    {
        m_UtilityCommandAllocators[i] = std::make_unique<DX12CommandAllocator>(
            m_Device->Get(),
            D3D12_COMMAND_LIST_TYPE_DIRECT); 
    }

    m_UtilityCommandList = std::make_unique<DX12CommandList>(
        m_Device->Get(),
        m_UtilityCommandAllocators[0]->Get(),
        D3D12_COMMAND_LIST_TYPE_DIRECT);
}

void GfxRenderer::InitGuiManager()
{
    m_ImGui.Initialize(*m_Context, *m_SRVDescriptorHeap);
}

void GfxRenderer::ResetUtilityCommandLists()
{
    m_UtilityCommandAllocators[m_SwapChain->GetCurrentBackBufferIndex()]->Get()->Reset();
    m_UtilityCommandList->Get()->Reset(m_UtilityCommandAllocators[m_SwapChain->GetCurrentBackBufferIndex()]->Get().Get(), nullptr);
}

void GfxRenderer::ClearRenderTarget()
{
    // TODO: Clearing might be simple, but since we have a framework setup to build command lists
    // we should use that instead here.
    ethVector4 clearColorVec = m_Context->GetClearColor();
    float clearColor[] = { clearColorVec.x, clearColorVec.y, clearColorVec.z, clearColorVec.w };

    CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        m_SwapChain->GetCurrentBackBuffer().Get(),
        D3D12_RESOURCE_STATE_PRESENT,
        D3D12_RESOURCE_STATE_RENDER_TARGET
    );

    m_UtilityCommandList->Get()->ResourceBarrier(1, &barrier);

    auto rtv = CD3DX12_CPU_DESCRIPTOR_HANDLE(
        m_RTVDescriptorHeap->Get()->GetCPUDescriptorHandleForHeapStart(),
        m_SwapChain->GetCurrentBackBufferIndex(),
        m_RTVDescriptorSize);

    m_UtilityCommandList->Get()->ClearRenderTargetView(rtv, clearColor, 0, nullptr);

    // Immediately execute this command list.
    // Since this utility command list will not be used anymore
    ThrowIfFailed(m_UtilityCommandList->Get()->Close());

    ID3D12CommandList* const clearCommand[] = {
        m_UtilityCommandList->Get().Get()
    };

    m_DirectCommandQueue->Get()->ExecuteCommandLists(_countof(clearCommand), clearCommand);

    // We want to signal and wait for the fence because we don't want issue more commands
    // to the utility command list while it is in flight. This is fine in this case because
    // clearing *should* be very fast on the GPU. TODO: Profile
    m_DirectCommandQueue->Signal(*m_Fence, m_Fence->Increment());
    m_FenceValues[m_SwapChain->GetCurrentBackBufferIndex()] = m_Fence->GetValue();
    WaitForGPU();
    
    // Reset the utility command list for others to use
    ResetUtilityCommandLists();
}

//void GfxRenderer::RenderKMS()
//{
//    const ethXMVector eyePosition = DirectX::XMVectorSet(0, 0, -5, 1);
//    const ethXMVector focusPoint = DirectX::XMVectorSet(0, 0, 0, 1);
//    const ethXMVector upDirection = DirectX::XMVectorSet(0, 1, 0, 0);
//    ethXMMatrix view = DirectX::XMMatrixLookAtLH(eyePosition, focusPoint, upDirection);
//
//    // Update the projection matrix.
//    float aspectRatio = m_Engine->GetEngineConfig().GetClientWidth() / static_cast<float>(m_Engine->GetEngineConfig().GetClientHeight());
//    ethXMMatrix projection = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(70), aspectRatio, 0.1f, 100.0f);
//
//    ethXMMatrix mvpMatrix = DirectX::XMMatrixMultiply(model, view);
//    mvpMatrix = DirectX::XMMatrixMultiply(mvpMatrix, projection);
//
//}

void GfxRenderer::RenderGui()
{
    if (!m_ImGui.GetVisible())
        return;

    SetRTCommonParams(*m_UtilityCommandList);

    ID3D12DescriptorHeap* srvdescHeap = m_SRVDescriptorHeap->Get().Get();
    m_UtilityCommandList->Get()->SetDescriptorHeaps(1, &srvdescHeap);

    m_ImGui.Render(*m_UtilityCommandList);
}

void GfxRenderer::Present()
{
    CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        m_SwapChain->GetCurrentBackBuffer().Get(),
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        D3D12_RESOURCE_STATE_PRESENT
    );

    m_UtilityCommandList->Get()->ResourceBarrier(1, &barrier);

    ThrowIfFailed(m_UtilityCommandList->Get()->Close());

    ID3D12CommandList* const commands[] = {
        m_UtilityCommandList->Get().Get()
    };

    m_DirectCommandQueue->Get()->ExecuteCommandLists(_countof(commands), commands);

    // TODO: Implement VSync and tearing support
    ThrowIfFailed(m_SwapChain->Get()->Present(0, 0));

    m_DirectCommandQueue->Signal(*m_Fence, m_Fence->Increment());
    m_FenceValues[m_SwapChain->GetCurrentBackBufferIndex()] = m_Fence->GetValue();
}

void GfxRenderer::EndOfFrame()
{
    m_SwapChain->UpdateBackBufferIndex();
    WaitForGPU();
}

void GfxRenderer::WaitForGPU()
{
    m_Fence->WaitForFenceValue(m_FenceValues[m_SwapChain->GetCurrentBackBufferIndex()]);
}

void GfxRenderer::EnableDebugLayer()
{
#if defined(_DEBUG)
    // Always enable the debug layer before doing anything DX12 related
    // so all possible errors generated while creating DX12 objects
    // are caught by the debug layer.
    wrl::ComPtr<ID3D12Debug> debugInterface;
    ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface)));
    debugInterface->EnableDebugLayer();
#endif
}

DX12CommandQueue* GfxRenderer::GetCommandQueue(D3D12_COMMAND_LIST_TYPE type) const
{
    switch (type)
    {
    case D3D12_COMMAND_LIST_TYPE_DIRECT:
        return m_DirectCommandQueue.get();
    case D3D12_COMMAND_LIST_TYPE_COMPUTE:
        return m_ComputeCommandQueue.get();
    case D3D12_COMMAND_LIST_TYPE_COPY:
        return m_CopyCommandQueue.get();
    default:
        assert(false && "The requested command queue type is not yet supported.");
        return nullptr;
    }
}

/*
void GfxRenderer::UpdateBufferResource(
    wrl::ComPtr<ID3D12GraphicsCommandList2> commandList,
    ID3D12Resource** pDestinationResource,
    ID3D12Resource** pIntermediateResource,
    size_t numElements, size_t elementSize, const void* bufferData,
    D3D12_RESOURCE_FLAGS flags)
{
    size_t bufferSize = numElements * elementSize;

    // Create a committed resource for the GPU resource in a default heap.
    ThrowIfFailed(m_Device->Get()->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(bufferSize, flags),
        D3D12_RESOURCE_STATE_COPY_DEST,
        nullptr,
        IID_PPV_ARGS(pDestinationResource)));
        IID_PPV_ARGS(pDestinationResource)));

    // Create an committed resource for the upload.
    if (bufferData)
    {
    ThrowIfFailed(m_Device->Get()->CreateCommittedResource(
            &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
            D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer(bufferSize),
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(pIntermediateResource)));

        D3D12_SUBRESOURCE_DATA subresourceData = {};
        subresourceData.pData = bufferData;
        subresourceData.RowPitch = bufferSize;
        subresourceData.SlicePitch = subresourceData.RowPitch;

        UpdateSubresources(commandList.Get(),
            *pDestinationResource, *pIntermediateResource,
            0, 0, 1, &subresourceData);
    }
}


void GfxRenderer::LoadContent()
{
    auto commandQueue = GetCommandQueue(D3D12_COMMAND_LIST_TYPE_COPY);
    m_CopyCommandAllocators[m_SwapChain->GetCurrentBackBufferIndex()]->Get()->Reset();
    m_CopyCommandList->Get()->Reset(m_CopyCommandAllocators[m_SwapChain->GetCurrentBackBufferIndex()]->Get().Get(), nullptr);
    wrl::ComPtr<ID3D12Resource> intermediateVertexBuffer;
    UpdateBufferResource(m_CopyCommandList->Get(),
        &m_VertexBuffer, &intermediateVertexBuffer,
        _countof(g_Vertices), sizeof(VertexPosColor), g_Vertices);

    wrl::ComPtr<ID3D12Resource> intermediateIndexBuffer;
    UpdateBufferResource(m_CopyCommandList->Get(),
        &m_IndexBuffer, &intermediateIndexBuffer,
        _countof(g_Indicies), sizeof(WORD), g_Indicies);

    // Create root signature
    D3D12_FEATURE_DATA_ROOT_SIGNATURE featureData = {};
    featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;
        if (FAILED(m_Device->Get()->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &featureData, sizeof(featureData))))
    {
        featureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
    }

    // Allow input layout and deny unnecessary access to certain pipeline stages.
    D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
        D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;

    // A single 32-bit constant root parameter that is used by the vertex shader.
    CD3DX12_ROOT_PARAMETER1 rootParameters[1];
    rootParameters[0].InitAsConstants(sizeof(ethMatrix4x4) / 4, 0, 0, D3D12_SHADER_VISIBILITY_VERTEX);

    CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDescription;
    rootSignatureDescription.Init_1_1(_countof(rootParameters), rootParameters, 0, nullptr, rootSignatureFlags);

    // Serialize the root signature.
    wrl::ComPtr<ID3DBlob> rootSignatureBlob;
    wrl::ComPtr<ID3DBlob> errorBlob;
    ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&rootSignatureDescription,
        featureData.HighestVersion, &rootSignatureBlob, &errorBlob));
    // Create the root signature.
    ThrowIfFailed(m_Device->Get()->CreateRootSignature(0, rootSignatureBlob->GetBufferPointer(),
        rootSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&m_RootSignature)));
    CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        m_SwapChain->GetCurrentBackBuffer().Get(),
        D3D12_RESOURCE_STATE_PRESENT,
        D3D12_RESOURCE_STATE_RENDER_TARGET
    );

    m_UtilityCommandList->Get()->ResourceBarrier(1, &barrier);

    wrl::ComPtr<ID3DBlob> vertexShader;
    wrl::ComPtr<ID3DBlob> pixelShader;
    wrl::ComPtr<ID3DBlob> errorMessage;

#if defined(_DEBUG)
    // Enable better shader debugging with the graphics debugging tools.
    UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
    UINT compileFlags = 0;
#endif

    if (FAILED(D3DCompileFromFile(
        L"data/shader/default_vs.hlsl",
        nullptr,
        nullptr,
        "VS_Main",
        "vs_5_1",
        compileFlags,
        0,
        &vertexShader,
        &errorMessage)) && errorMessage)
    {
        const char* err = (const char*)errorMessage->GetBufferPointer();
        OutputDebugStringA(err);
    }

    if (FAILED(D3DCompileFromFile(
        L"data/shader/default_ps.hlsl",
        nullptr,
        nullptr,
        "PS_Main",
        "ps_5_1",
        compileFlags,
        0,
        &pixelShader,
        &errorMessage)) && errorMessage)
    {
        const char* err = (const char*)errorMessage->GetBufferPointer();
        OutputDebugStringA(err);
    }

    // Define the vertex input layout.
    D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };

    // Create PSO Desc
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
    psoDesc.pRootSignature = m_RootSignature.Get();
    psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
    psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    psoDesc.DepthStencilState.DepthEnable = FALSE;
    psoDesc.DepthStencilState.StencilEnable = FALSE;
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.SampleDesc.Count = 1;

    // Compile and create the actual PSO
    m_PipelineState = std::make_unique<DX12PipelineState>(m_Device->Get(), psoDesc);


    // A D3D12_VERTEX_BUFFER_VIEW structure is used to tell the Input Assembler stage
    // where the vertices are stored in GPU memory.
    m_VertexBufferView.BufferLocation = m_VertexBuffer->GetGPUVirtualAddress();
    m_VertexBufferView.SizeInBytes = sizeof(g_Vertices);
    m_VertexBufferView.StrideInBytes = sizeof(VertexPosColor);

    m_IndexBufferView.BufferLocation = m_IndexBuffer->GetGPUVirtualAddress();
    m_IndexBufferView.Format = DXGI_FORMAT_R16_UINT;
    m_IndexBufferView.SizeInBytes = sizeof(g_Indicies);

    m_CopyCommandList->Get()->Close();

    ID3D12CommandList* const commandLists[] = {
        m_CopyCommandList->Get().Get()
    };

    m_CopyCommandQueue->Get()->ExecuteCommandLists(_countof(commandLists), commandLists);
    m_CopyCommandQueue->Signal(*m_Fence, m_Fence->Increment());
    WaitForGPU();
}
*/

