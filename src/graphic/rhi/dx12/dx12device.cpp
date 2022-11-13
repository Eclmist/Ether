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

#include "dx12device.h"
#include "dx12commandallocator.h"
#include "dx12commandlist.h"
#include "dx12commandqueue.h"
#include "dx12descriptorheap.h"
#include "dx12fence.h"
#include "dx12pipelinestate.h"
#include "dx12resource.h"
#include "dx12resourceviews.h"
#include "dx12rootparameter.h"
#include "dx12rootsignature.h"
#include "dx12swapchain.h"

#include "dx12translation.h"

ETH_NAMESPACE_BEGIN

RhiResult Dx12Device::CreateCommandAllocator(const RhiCommandAllocatorDesc& desc, RhiCommandAllocatorHandle& allocator) const
{
    Dx12CommandAllocator* d3dCommandAllocator = new Dx12CommandAllocator();

    HRESULT hr = m_Device->CreateCommandAllocator(
        Translate(desc.m_Type),
        IID_PPV_ARGS(&d3dCommandAllocator->m_Allocator)
    );

    d3dCommandAllocator->m_Allocator->SetName(allocator.GetName().c_str());
    allocator.Set(d3dCommandAllocator);
    return TO_RHI_RESULT(hr);
}

RhiResult Dx12Device::CreateCommandList(const RhiCommandListDesc& desc, RhiCommandListHandle& cmdList) const
{
    Dx12CommandList* d3dCommandList = new Dx12CommandList(desc.m_Type);
    const auto allocator = desc.m_Allocator.As<Dx12CommandAllocator>();

    HRESULT hr = m_Device->CreateCommandList(
        1,
        Translate(desc.m_Type),
        allocator->m_Allocator.Get(),
        nullptr,
        IID_PPV_ARGS(&d3dCommandList->m_CommandList)
    );

    d3dCommandList->m_CommandList->SetName(cmdList.GetName().c_str());
    cmdList.Set(d3dCommandList);
    return TO_RHI_RESULT(hr);
}

RhiResult Dx12Device::CreateCommandQueue(const RhiCommandQueueDesc& desc, RhiCommandQueueHandle& cmdQueue) const
{
    Dx12CommandQueue* d3dCommandQueue = new Dx12CommandQueue(desc.m_Type);

    HRESULT hr = m_Device->CreateCommandQueue(
        &Translate(desc),
        IID_PPV_ARGS(&d3dCommandQueue->m_CommandQueue)
    );

    d3dCommandQueue->m_CommandQueue->SetName(cmdQueue.GetName().c_str());
    cmdQueue.Set(d3dCommandQueue);
    return TO_RHI_RESULT(hr);
}

RhiResult Dx12Device::CreateDescriptorHeap(const RhiDescriptorHeapDesc& desc, RhiDescriptorHeapHandle& descriptorHeap) const
{
    Dx12DescriptorHeap* d3dDescriptorHeap = new Dx12DescriptorHeap();

    HRESULT hr = m_Device->CreateDescriptorHeap(
        &Translate(desc),
        IID_PPV_ARGS(&d3dDescriptorHeap->m_Heap)
    );

    d3dDescriptorHeap->m_Heap->SetName(descriptorHeap.GetName().c_str());
    d3dDescriptorHeap->m_HandleIncrementSize = m_Device->GetDescriptorHandleIncrementSize(Translate(desc.m_Type));
    descriptorHeap.Set(d3dDescriptorHeap);
    return RhiResult();
}

RhiResult Dx12Device::CreateFence(RhiFenceHandle& fence) const
{
    Dx12Fence* d3dFence = new Dx12Fence();
    HRESULT hr = m_Device->CreateFence(
        0,
        D3D12_FENCE_FLAG_NONE,
        IID_PPV_ARGS(&d3dFence->m_Fence)
    );

    d3dFence->m_Fence->SetName(fence.GetName().c_str());
    fence.Set(d3dFence);
    return TO_RHI_RESULT(hr);
}

RhiResult Dx12Device::CreatePipelineState(const RhiPipelineStateDesc& desc, RhiPipelineStateHandle& pipelineState) const
{
    Dx12PipelineState* d3dPipelineState = new Dx12PipelineState();
    const auto rootSignature = desc.m_RootSignature.As<Dx12RootSignature>();

    D3D12_INPUT_LAYOUT_DESC inputLayout = {};

    for (int i = 0; i < desc.m_InputLayoutDesc.m_NumElements; ++i)
        d3dPipelineState->m_InputElements.push_back(Translate(desc.m_InputLayoutDesc.m_InputElementDescs[i]));

    inputLayout.NumElements = desc.m_InputLayoutDesc.m_NumElements;
    inputLayout.pInputElementDescs = d3dPipelineState->m_InputElements.data();

    D3D12_GRAPHICS_PIPELINE_STATE_DESC d3dDesc = Translate(desc);
    d3dDesc.pRootSignature = rootSignature->m_RootSignature.Get();
    d3dDesc.InputLayout = inputLayout;

    HRESULT hr = m_Device->CreateGraphicsPipelineState(
        &d3dDesc, 
        IID_PPV_ARGS(&d3dPipelineState->m_PipelineState)
    );

    d3dPipelineState->m_PipelineState->SetName(pipelineState.GetName().c_str());
    pipelineState.Set(d3dPipelineState);
    return TO_RHI_RESULT(hr);
}

RhiResult Dx12Device::CreateRootParameter(RhiRootParameterHandle& rootParameter) const
{
    D3D12RootParameter* d3dRootParameter = new D3D12RootParameter();
    rootParameter.Set(d3dRootParameter);
    return RhiResult::Success;
}

RhiResult Dx12Device::CreateRootSignature(const RhiRootSignatureDesc& desc, RhiRootSignatureHandle& rootSignature) const
{
    wrl::ComPtr<ID3DBlob> rsBlob, errBlob;
    Dx12RootSignature* d3dRootSignature = new Dx12RootSignature();

    for (int i = 0; i < desc.m_NumParameters; ++i)
        d3dRootSignature->m_D3DRootParameters.push_back(desc.m_Parameters[i].As<D3D12RootParameter>()->m_Parameter);

    for (int i = 0; i < desc.m_NumStaticSamplers; ++i)
        d3dRootSignature->m_D3DStaticSamplers.push_back(Translate(desc.m_StaticSamplers[i]));

    D3D12_ROOT_SIGNATURE_DESC d3dDesc = Translate(desc);
    d3dDesc.pParameters = d3dRootSignature->m_D3DRootParameters.data();
    d3dDesc.pStaticSamplers = d3dRootSignature->m_D3DStaticSamplers.data();

    ASSERT_SUCCESS(D3D12SerializeRootSignature(&d3dDesc, D3D_ROOT_SIGNATURE_VERSION_1,
        rsBlob.GetAddressOf(), errBlob.GetAddressOf()), errBlob->GetBufferPointer());

    HRESULT hr = m_Device->CreateRootSignature(
        1,
        rsBlob->GetBufferPointer(),
        rsBlob->GetBufferSize(),
        IID_PPV_ARGS(&d3dRootSignature->m_RootSignature)
    );

    d3dRootSignature->m_RootSignature->SetName(rootSignature.GetName().c_str());
    rootSignature.Set(d3dRootSignature);
    return TO_RHI_RESULT(hr);
}

RhiResult Dx12Device::CreateSwapChain(const RhiSwapChainDesc& desc, RhiSwapChainHandle& swapChain) const
{
    Dx12SwapChain* d3dSwapChain = new Dx12SwapChain();
    const auto d3dCommandQueue = desc.m_CommandQueue.As<Dx12CommandQueue>();

    wrl::ComPtr<IDXGIFactory4> dxgiFactory;
    wrl::ComPtr<IDXGISwapChain1> swapChain1;
    ASSERT_SUCCESS(CreateDXGIFactory2(0, IID_PPV_ARGS(&dxgiFactory)));
    ASSERT_SUCCESS(dxgiFactory->MakeWindowAssociation((HWND)desc.m_WindowHandle, DXGI_MWA_NO_ALT_ENTER));

    HRESULT hr = dxgiFactory->CreateSwapChainForHwnd(
        d3dCommandQueue->m_CommandQueue.Get(),
        (HWND)desc.m_WindowHandle,
        &Translate(desc),
        nullptr,
        nullptr,
        &swapChain1
    );

    swapChain1.As(&d3dSwapChain->m_SwapChain);

    swapChain.Set(d3dSwapChain);
    return TO_RHI_RESULT(hr);
}

RhiResult Dx12Device::CreateRenderTargetView(const RhiRenderTargetViewDesc& desc, RhiRenderTargetViewHandle& rtvHandle) const
{
    Dx12RenderTargetView* d3dRtv = new Dx12RenderTargetView();
    d3dRtv->m_CpuHandle = desc.m_TargetCpuHandle;

    const auto d3dResource = desc.m_Resource.As<Dx12Resource>();

    m_Device->CreateRenderTargetView(
        d3dResource->m_Resource.Get(),
        &Translate(desc),
        Translate(d3dRtv->m_CpuHandle)
    );

    rtvHandle.Set(d3dRtv);
    return RhiResult::Success;
}

RhiResult Dx12Device::CreateDepthStencilView(const RhiDepthStencilViewDesc& desc, RhiDepthStencilViewHandle& dsvHandle) const
{
    D3D12DepthStencilView* d3dDsv = new D3D12DepthStencilView();
    d3dDsv->m_CpuHandle = desc.m_TargetCpuHandle;

    const auto d3dResource = desc.m_Resource.As<Dx12Resource>();

    m_Device->CreateDepthStencilView(
        d3dResource->m_Resource.Get(),
        &Translate(desc),
        Translate(d3dDsv->m_CpuHandle)
    );

    dsvHandle.Set(d3dDsv);
    return RhiResult::Success;
}

RhiResult Dx12Device::CreateShaderResourceView(const RhiShaderResourceViewDesc& desc, RhiShaderResourceViewHandle& srvHandle) const
{
    D3D12ShaderResourceView* d3dSrv = new D3D12ShaderResourceView();
    d3dSrv->m_CpuHandle = desc.m_TargetCpuHandle;
    d3dSrv->m_GpuHandle = desc.m_TargetGpuHandle;

    const auto d3dResource = desc.m_Resource.As<Dx12Resource>();

    m_Device->CreateShaderResourceView(
        d3dResource->m_Resource.Get(),
        &Translate(desc),
        Translate(d3dSrv->m_CpuHandle)
    );

    srvHandle.Set(d3dSrv);
    return RhiResult::Success;
}

RhiResult Dx12Device::CreateConstantBufferView(const RhiConstantBufferViewDesc& desc, RhiConstantBufferViewHandle& cbvHandle) const
{
    D3D12ConstantBufferView* d3dCbv = new D3D12ConstantBufferView();
    d3dCbv->m_CpuHandle = desc.m_TargetCpuHandle;
    d3dCbv->m_GpuHandle = desc.m_TargetGpuHandle;

    const auto d3dResource = desc.m_Resource.As<Dx12Resource>();

    m_Device->CreateConstantBufferView(
        &Translate(desc),
        Translate(d3dCbv->m_CpuHandle)
    );

    cbvHandle.Set(d3dCbv);
    return RhiResult::Success;
}

RhiResult Dx12Device::CreateUnorderedAccessView(const RhiUnorderedAccessViewDesc& desc, RhiUnorderedAccessViewHandle& uavHandle) const
{
    return RhiResult::NotSupported;
}

RhiResult Dx12Device::CreateCommittedResource(const RhiCommitedResourceDesc& desc, RhiResourceHandle& resourceHandle) const
{
    Dx12Resource* d3dResource = new Dx12Resource();

    HRESULT hr = m_Device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(Translate(desc.m_HeapType)),
        D3D12_HEAP_FLAG_NONE,
        &Translate(desc.m_ResourceDesc),
        Translate(desc.m_State),
        desc.m_ClearValue == nullptr ? nullptr : &Translate(*desc.m_ClearValue),
        IID_PPV_ARGS(&d3dResource->m_Resource)
    );

    d3dResource->m_Resource->SetName(resourceHandle.GetName().c_str());
    resourceHandle.Set(d3dResource);
    return TO_RHI_RESULT(hr);
}

ETH_NAMESPACE_END

