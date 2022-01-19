/*
    This file is part of Ether, an open-source DirectX 12 renderer.

    Copyright (c) 2020-2022 Samuel Huang - All rights reserved.

    Ether is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "d3d12device.h"
#include "d3d12commandallocator.h"
#include "d3d12commandlist.h"
#include "d3d12commandqueue.h"
#include "d3d12descriptorheap.h"
#include "d3d12fence.h"
#include "d3d12pipelinestate.h"
#include "d3d12resource.h"
#include "d3d12resourceviews.h"
#include "d3d12rootparameter.h"
#include "d3d12rootsignature.h"
#include "d3d12swapchain.h"

#include "d3d12translation.h"

ETH_NAMESPACE_BEGIN

RHIResult D3D12Device::CreateCommandAllocator(const RHICommandAllocatorDesc& desc, RHICommandAllocatorHandle& allocator) const
{
    D3D12CommandAllocator* d3dCommandAllocator = new D3D12CommandAllocator();

    HRESULT hr = m_Device->CreateCommandAllocator(
        Translate(desc.m_Type),
        IID_PPV_ARGS(&d3dCommandAllocator->m_Allocator)
    );

    d3dCommandAllocator->m_Allocator->SetName(allocator.GetName());
    allocator.Set(d3dCommandAllocator);
    return TO_RHI_RESULT(hr);
}

RHIResult D3D12Device::CreateCommandList(const RHICommandListDesc& desc, RHICommandListHandle& cmdList) const
{
    D3D12CommandList* d3dCommandList = new D3D12CommandList(desc.m_Type);
    const auto allocator = desc.m_Allocator.As<D3D12CommandAllocator>();

    HRESULT hr = m_Device->CreateCommandList(
		1,
        Translate(desc.m_Type),
		allocator->m_Allocator.Get(),
		nullptr,
		IID_PPV_ARGS(&d3dCommandList->m_CommandList)
    );

    d3dCommandList->m_CommandList->SetName(cmdList.GetName());
    cmdList.Set(d3dCommandList);
    return TO_RHI_RESULT(hr);
}

RHIResult D3D12Device::CreateCommandQueue(const RHICommandQueueDesc& desc, RHICommandQueueHandle& cmdQueue) const
{
    D3D12CommandQueue* d3dCommandQueue = new D3D12CommandQueue(desc.m_Type);

    HRESULT hr = m_Device->CreateCommandQueue(
        &Translate(desc),
        IID_PPV_ARGS(&d3dCommandQueue->m_CommandQueue)
    );

    d3dCommandQueue->m_CommandQueue->SetName(cmdQueue.GetName());
    cmdQueue.Set(d3dCommandQueue);
    return TO_RHI_RESULT(hr);
}

RHIResult D3D12Device::CreateDescriptorHeap(const RHIDescriptorHeapDesc& desc, RHIDescriptorHeapHandle& descriptorHeap) const
{
    D3D12DescriptorHeap* d3dDescriptorHeap = new D3D12DescriptorHeap();

    HRESULT hr = m_Device->CreateDescriptorHeap(
        &Translate(desc),
        IID_PPV_ARGS(&d3dDescriptorHeap->m_Heap)
    );

    d3dDescriptorHeap->m_Heap->SetName(descriptorHeap.GetName());
    d3dDescriptorHeap->m_HandleIncrementSize = m_Device->GetDescriptorHandleIncrementSize(Translate(desc.m_Type));
    descriptorHeap.Set(d3dDescriptorHeap);
    return RHIResult();
}

RHIResult D3D12Device::CreateFence(RHIFenceHandle& fence) const
{
    D3D12Fence* d3dFence = new D3D12Fence();
    HRESULT hr = m_Device->CreateFence(
        0,
        D3D12_FENCE_FLAG_NONE,
        IID_PPV_ARGS(&d3dFence->m_Fence)
    );

    d3dFence->m_Fence->SetName(fence.GetName());
    fence.Set(d3dFence);
    return TO_RHI_RESULT(hr);
}

RHIResult D3D12Device::CreatePipelineState(const RHIPipelineStateDesc& desc, RHIPipelineStateHandle& pipelineState) const
{
    D3D12PipelineState* d3dPipelineState = new D3D12PipelineState();
    const auto rootSignature = desc.m_RootSignature.As<D3D12RootSignature>();

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

    d3dPipelineState->m_PipelineState->SetName(pipelineState.GetName());
    pipelineState.Set(d3dPipelineState);
    return TO_RHI_RESULT(hr);
}

RHIResult D3D12Device::CreateRootParameter(RHIRootParameterHandle& rootParameter) const
{
    D3D12RootParameter* d3dRootParameter = new D3D12RootParameter();
    rootParameter.Set(d3dRootParameter);
    return RHIResult::Success;
}

RHIResult D3D12Device::CreateRootSignature(const RHIRootSignatureDesc& desc, RHIRootSignatureHandle& rootSignature) const
{
    wrl::ComPtr<ID3DBlob> rsBlob, errBlob;
    D3D12RootSignature* d3dRootSignature = new D3D12RootSignature();

    for (int i = 0; i < desc.m_NumParameters; ++i)
        d3dRootSignature->m_D3DRootParameters.push_back(desc.m_Parameters[i].As<D3D12RootParameter>()->m_Parameter);

    for (int i = 0; i < desc.m_NumStaticSamplers; ++i)
        d3dRootSignature->m_D3DStaticSamplers.push_back(Translate(desc.m_StaticSamplers[i]));

    D3D12_ROOT_SIGNATURE_DESC d3dDesc = Translate(desc);
    d3dDesc.pParameters = d3dRootSignature->m_D3DRootParameters.data();
    d3dDesc.pStaticSamplers = d3dRootSignature->m_D3DStaticSamplers.data();

    ASSERT_SUCCESS(D3D12SerializeRootSignature(&d3dDesc, D3D_ROOT_SIGNATURE_VERSION_1,
        rsBlob.GetAddressOf(), errBlob.GetAddressOf()));

    HRESULT hr = m_Device->CreateRootSignature(
        1,
        rsBlob->GetBufferPointer(),
        rsBlob->GetBufferSize(),
        IID_PPV_ARGS(&d3dRootSignature->m_RootSignature)
    );

    d3dRootSignature->m_RootSignature->SetName(rootSignature.GetName());
    rootSignature.Set(d3dRootSignature);
    return TO_RHI_RESULT(hr);
}

RHIResult D3D12Device::CreateSwapChain(const RHISwapChainDesc& desc, RHISwapChainHandle& swapChain) const
{
    D3D12SwapChain* d3dSwapChain = new D3D12SwapChain();
    const auto d3dCommandQueue = desc.m_CommandQueue.As<D3D12CommandQueue>();

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

RHIResult D3D12Device::CreateRenderTargetView(const RHIRenderTargetViewDesc& desc, RHIRenderTargetViewHandle& rtvHandle) const
{
    D3D12RenderTargetView* d3dRtv = new D3D12RenderTargetView();
    d3dRtv->m_CpuHandle = GraphicCore::GetRTVDescriptorHeap()->GetNextHandleCPU();
    GraphicCore::GetRTVDescriptorHeap()->IncrementHandle();

    const auto d3dResource = desc.m_Resource.As<D3D12Resource>();

    m_Device->CreateRenderTargetView(
        d3dResource->m_Resource.Get(),
        &Translate(desc),
        Translate(d3dRtv->m_CpuHandle)
    );

	rtvHandle.Set(d3dRtv);
    return RHIResult::Success;
}

RHIResult D3D12Device::CreateDepthStencilView(const RHIDepthStencilViewDesc& desc, RHIDepthStencilViewHandle& dsvHandle) const
{
    D3D12DepthStencilView* d3dDsv = new D3D12DepthStencilView();
    d3dDsv->m_CpuHandle = GraphicCore::GetDSVDescriptorHeap()->GetNextHandleCPU();
    GraphicCore::GetDSVDescriptorHeap()->IncrementHandle();

    const auto d3dResource = desc.m_Resource.As<D3D12Resource>();

    m_Device->CreateDepthStencilView(
        d3dResource->m_Resource.Get(),
        &Translate(desc),
        Translate(d3dDsv->m_CpuHandle)
    );

    dsvHandle.Set(d3dDsv);
    return RHIResult::Success;
}

RHIResult D3D12Device::CreateShaderResourceView(const RHIShaderResourceViewDesc& desc, RHIShaderResourceViewHandle& srvHandle) const
{
    D3D12ShaderResourceView* d3dSrv = new D3D12ShaderResourceView();
    d3dSrv->m_CpuHandle = GraphicCore::GetSRVDescriptorHeap()->GetNextHandleCPU();
    d3dSrv->m_GpuHandle = GraphicCore::GetSRVDescriptorHeap()->GetNextHandleGPU();
    GraphicCore::GetSRVDescriptorHeap()->IncrementHandle();

    const auto d3dResource = desc.m_Resource.As<D3D12Resource>();

    m_Device->CreateShaderResourceView(
        d3dResource->m_Resource.Get(),
        &Translate(desc),
        Translate(d3dSrv->m_CpuHandle)
    );

    srvHandle.Set(d3dSrv);
    return RHIResult::Success;
}

RHIResult D3D12Device::CreateConstantBufferView(const RHIConstantBufferViewDesc& desc, RHIConstantBufferViewHandle& cbvHandle) const
{
    return RHIResult::NotSupported;
}

RHIResult D3D12Device::CreateUnorderedAccessView(const RHIUnorderedAccessViewDesc& desc, RHIUnorderedAccessViewHandle& uavHandle) const
{
    return RHIResult::NotSupported;
}

RHIResult D3D12Device::CreateCommittedResource(const RHICommitedResourceDesc& desc, RHIResourceHandle& resourceHandle) const
{
    D3D12Resource* d3dResource = new D3D12Resource();

    HRESULT hr = m_Device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(Translate(desc.m_HeapType)),
        D3D12_HEAP_FLAG_NONE,
        &Translate(desc.m_ResourceDesc),
        Translate(desc.m_State),
        desc.m_ClearValue == nullptr ? nullptr : &Translate(*desc.m_ClearValue),
        IID_PPV_ARGS(&d3dResource->m_Resource)
    );

    d3dResource->m_Resource->SetName(resourceHandle.GetName());
    resourceHandle.Set(d3dResource);
    return TO_RHI_RESULT(hr);
}

ETH_NAMESPACE_END

