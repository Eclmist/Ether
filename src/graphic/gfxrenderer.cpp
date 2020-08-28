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
#include "engine/engine.h"
#include "system/win32/window.h"

// Vertex data for a colored cube.
struct VertexPosColor
{
    ethVector3 Position;
    ethVector3 Color;
};

static VertexPosColor g_Vertices[8] = {
    { ethVector3(-1.0f, -1.0f, -1.0f), ethVector3(0.0f, 0.0f, 0.0f) }, // 0
    { ethVector3(-1.0f, 1.0f, -1.0f), ethVector3(0.0f, 1.0f, 0.0f) }, // 1
    { ethVector3(1.0f, 1.0f, -1.0f), ethVector3(1.0f, 1.0f, 0.0f) }, // 2
    { ethVector3(1.0f, -1.0f, -1.0f), ethVector3(1.0f, 0.0f, 0.0f) }, // 3
    { ethVector3(-1.0f, -1.0f, 1.0f), ethVector3(0.0f, 0.0f, 1.0f) }, // 4
    { ethVector3(-1.0f, 1.0f, 1.0f), ethVector3(0.0f, 1.0f, 1.0f) }, // 5
    { ethVector3(1.0f, 1.0f, 1.0f), ethVector3(1.0f, 1.0f, 1.0f) }, // 6
    { ethVector3(1.0f, -1.0f, 1.0f), ethVector3(1.0f, 0.0f, 1.0f) }
};

static WORD g_Indicies[36] =
{
    0, 1, 2, 0, 2, 3,
    4, 6, 5, 4, 7, 6,
    4, 5, 1, 4, 1, 0,
    3, 2, 6, 3, 6, 7,
    1, 5, 6, 1, 6, 2,
    4, 0, 3, 4, 3, 7
};


GfxRenderer::GfxRenderer(Engine* engine)
    : EngineSubsystem(engine)
{
    m_Viewport = CD3DX12_VIEWPORT(0.0, 0.0f,
        static_cast<float>(engine->GetEngineConfig().GetClientWidth()),
        static_cast<float>(engine->GetEngineConfig().GetClientHeight()));

    m_ScissorRect = CD3DX12_RECT(0, 0, LONG_MAX, LONG_MAX);

}

void GfxRenderer::Initialize()
{
    EnableDebugLayer();

    m_Adapter = std::make_unique<DX12Adapter>();
    m_Device = std::make_unique<DX12Device>(m_Adapter->Get());
    m_Context = std::make_unique<GfxContext>(*m_Device, *this);

    m_DirectCommandQueue = std::make_unique<DX12CommandQueue>(m_Device->Get(), D3D12_COMMAND_LIST_TYPE_DIRECT);
    m_ComputeCommandQueue = std::make_unique<DX12CommandQueue>(m_Device->Get(), D3D12_COMMAND_LIST_TYPE_COMPUTE);
    m_CopyCommandQueue = std::make_unique<DX12CommandQueue>(m_Device->Get(), D3D12_COMMAND_LIST_TYPE_COPY);

    Window* window = m_Engine->GetWindow();

    m_SwapChain = std::make_unique<DX12SwapChain>(
        window->GetHwnd(),
        m_Device->Get(),
        m_DirectCommandQueue->Get(),
        m_Engine->GetEngineConfig().GetClientWidth(),
        m_Engine->GetEngineConfig().GetClientHeight());

    m_RTVDescriptorHeap = std::make_unique<DX12DescriptorHeap>(
        m_Device->Get(),
        D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
        ETH_NUM_SWAPCHAIN_BUFFERS
    );

    m_RTVDescriptorSize = m_Device->Get()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    m_SwapChain->CreateRenderTargetViews(m_Device->Get(), m_RTVDescriptorHeap->Get());
    m_Fence = std::make_unique<DX12Fence>(m_Device->Get());
    
    for (int i = 0; i < ETH_NUM_SWAPCHAIN_BUFFERS; ++i)
    {
        m_CommandAllocators[i] = std::make_unique<DX12CommandAllocator>(m_Device->Get(), D3D12_COMMAND_LIST_TYPE_DIRECT); 
        m_CopyCommandAllocators[i] = std::make_unique<DX12CommandAllocator>(m_Device->Get(), D3D12_COMMAND_LIST_TYPE_COPY); 
        m_FenceValues[i] = 0;
    }

    m_CommandList = std::make_unique<DX12CommandList>(m_Device->Get(), m_CommandAllocators[0]->Get(), D3D12_COMMAND_LIST_TYPE_DIRECT);
    m_CopyCommandList = std::make_unique<DX12CommandList>(m_Device->Get(), m_CopyCommandAllocators[0]->Get(), D3D12_COMMAND_LIST_TYPE_COPY);

    m_SRVDescriptorHeap = std::make_unique<DX12DescriptorHeap>(
        m_Device->Get(),
        D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
        1,
        D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE
    );

    m_GfxImGui.Initialize(*m_Context, *m_SRVDescriptorHeap);

    SetInitialized(true);
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
    if (!IsInitialized())
        return;

    m_Timer.Update();

    ResetCommandList();
    ClearRenderTarget(); // For now, just clear the backbuffer to some color
    RenderKMS();
    RenderGui();
    Present();
    EndOfFrame();
}

void GfxRenderer::ToggleGui()
{
    m_GfxImGui.ToggleVisible();
}

void GfxRenderer::Resize(uint32_t width, uint32_t height)
{
    // Flush the graphics device to make sure that the swap chain buffers that we're
    // about to release are not referenced by any in-flight command lists
    Flush();

    // Buffers are released
    m_SwapChain->ResizeBuffers(width, height);

    // Recreate buffers with new size
    m_SwapChain->CreateRenderTargetViews(m_Device->Get(), m_RTVDescriptorHeap->Get());

    m_Viewport = CD3DX12_VIEWPORT(0.0f, 0.0f,
        static_cast<float>(width), static_cast<float>(height));

}

void GfxRenderer::ResetCommandList()
{
    m_CommandAllocators[m_SwapChain->GetCurrentBackBufferIndex()]->Get()->Reset();
    m_CommandList->Get()->Reset(m_CommandAllocators[m_SwapChain->GetCurrentBackBufferIndex()]->Get().Get(), nullptr);
}

void GfxRenderer::ClearRenderTarget()
{
    // Hardcode clear color for now
    //float clearColor[] = { 
    //    (float)sin(m_Timer.GetTimeSinceStart() * 0.0001f),
    //    (float)cos(m_Timer.GetTimeSinceStart() * 0.0002f),
    //    (float)sin(m_Timer.GetTimeSinceStart() * 0.00015f),
    //    1.0f };

    ethVector4 clearColorVec = m_Context->GetClearColor();
    float clearColor[] = { clearColorVec.x, clearColorVec.y, clearColorVec.z, clearColorVec.w };

    CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        m_SwapChain->GetCurrentBackBuffer().Get(),
        D3D12_RESOURCE_STATE_PRESENT, // Hard code before state for now
        D3D12_RESOURCE_STATE_RENDER_TARGET
    );

    m_CommandList->Get()->ResourceBarrier(1, &barrier);

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtv(
        m_RTVDescriptorHeap->Get()->GetCPUDescriptorHandleForHeapStart(),
        m_SwapChain->GetCurrentBackBufferIndex(),
        m_RTVDescriptorSize
    );

    m_CommandList->Get()->ClearRenderTargetView(rtv, clearColor, 0, nullptr);
}

void GfxRenderer::RenderKMS()
{
    // Update constant buffer values
    // Update the MVP matrix
    const ethXMVector rotationAxis = DirectX::XMVectorSet(0, 1, 1, 0);
    ethXMMatrix model = DirectX::XMMatrixRotationAxis(rotationAxis, static_cast<float>(m_Timer.GetTimeSinceStart() / 1000.0));
    const ethXMVector eyePosition = DirectX::XMVectorSet(0, 0, -5, 1);
    const ethXMVector focusPoint = DirectX::XMVectorSet(0, 0, 0, 1);
    const ethXMVector upDirection = DirectX::XMVectorSet(0, 1, 0, 0);
    ethXMMatrix view = DirectX::XMMatrixLookAtLH(eyePosition, focusPoint, upDirection);

    // Update the projection matrix.
    float aspectRatio = m_Engine->GetEngineConfig().GetClientWidth() / static_cast<float>(m_Engine->GetEngineConfig().GetClientHeight());
    ethXMMatrix projection = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(70), aspectRatio, 0.1f, 100.0f);

    ethXMMatrix mvpMatrix = DirectX::XMMatrixMultiply(model, view);
    mvpMatrix = DirectX::XMMatrixMultiply(mvpMatrix, projection);

    // Root signature must be explicitly set even though it is already set in the PSO
    // for some reason or other
    m_CommandList->Get()->SetGraphicsRootSignature(m_RootSignature.Get());

    m_CommandList->Get()->SetGraphicsRoot32BitConstants(0, sizeof(ethXMMatrix) / 4, &mvpMatrix, 0);

    // Bind the PSO
    m_CommandList->Get()->SetPipelineState(m_PipelineState->Get().Get());

    // Setup the input assembler
    m_CommandList->Get()->IASetPrimitiveTopology(m_Context->GetRenderWireframe() ? D3D_PRIMITIVE_TOPOLOGY_LINESTRIP : D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_CommandList->Get()->IASetVertexBuffers(0, 1, &m_VertexBufferView);
    m_CommandList->Get()->IASetIndexBuffer(&m_IndexBufferView);

    // Setup the rasterizer states
    m_CommandList->Get()->RSSetViewports(1, &m_Viewport);
    m_CommandList->Get()->RSSetScissorRects(1, &m_ScissorRect);

    // Bind render target to the output merger
    auto rtv = CD3DX12_CPU_DESCRIPTOR_HANDLE(m_RTVDescriptorHeap->Get()->GetCPUDescriptorHandleForHeapStart(),
        m_SwapChain->GetCurrentBackBufferIndex(), m_RTVDescriptorSize);
    m_CommandList->Get()->OMSetRenderTargets(1, &rtv, false, nullptr);

    m_CommandList->Get()->DrawIndexedInstanced(_countof(g_Indicies), 1, 0, 0, 0);
}

void GfxRenderer::RenderGui()
{
    if (!m_GfxImGui.GetVisible())
        return;

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtv(
        m_RTVDescriptorHeap->Get()->GetCPUDescriptorHandleForHeapStart(),
        m_SwapChain->GetCurrentBackBufferIndex(),
        m_RTVDescriptorSize
    );
    m_CommandList->Get()->OMSetRenderTargets(1, &(rtv), FALSE, NULL);
    ID3D12DescriptorHeap* srvdescHeap = m_SRVDescriptorHeap->Get().Get();
    m_CommandList->Get()->SetDescriptorHeaps(1, &srvdescHeap);

    m_GfxImGui.Render(*m_CommandList);
}

void GfxRenderer::Present()
{
    CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        m_SwapChain->GetCurrentBackBuffer().Get(),
        D3D12_RESOURCE_STATE_RENDER_TARGET, // TODO: Remove this. Hard code before state for now
        D3D12_RESOURCE_STATE_PRESENT
    );

    m_CommandList->Get()->ResourceBarrier(1, &barrier);

    ThrowIfFailed(m_CommandList->Get()->Close());

    ID3D12CommandList* const commandLists[] = {
        m_CommandList->Get().Get()
    };

    m_DirectCommandQueue->Get()->ExecuteCommandLists(_countof(commandLists), commandLists);

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
        L"../src/graphic/shaders/default_vs.hlsl",
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
        L"../src/graphic/shaders/default_ps.hlsl",
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

