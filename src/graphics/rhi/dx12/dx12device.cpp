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
#include "graphics/rhi/dx12/dx12device.h"
#include "graphics/rhi/dx12/dx12module.h"

#include "graphics/rhi/dx12/dx12commandallocator.h"
#include "graphics/rhi/dx12/dx12commandlist.h"
#include "graphics/rhi/dx12/dx12commandqueue.h"
#include "graphics/rhi/dx12/dx12descriptorheap.h"
#include "graphics/rhi/dx12/dx12fence.h"
#include "graphics/rhi/dx12/dx12pipelinestate.h"
#include "graphics/rhi/dx12/dx12resource.h"
#include "graphics/rhi/dx12/dx12resourceviews.h"
#include "graphics/rhi/dx12/dx12rootsignature.h"
#include "graphics/rhi/dx12/dx12swapchain.h"
#include "graphics/rhi/dx12/dx12shader.h"

#include "graphics/rhi/dx12/dx12translation.h"

#ifdef ETH_GRAPHICS_DX12

std::unique_ptr<Ether::Graphics::RhiRootSignatureDesc> Ether::Graphics::Dx12Device::CreateRootSignatureDesc(
    uint32_t numParams,
    uint32_t numSamplers) const
{
    return std::make_unique<Dx12RootSignatureDesc>(numParams, numSamplers);
}

std::unique_ptr<Ether::Graphics::RhiPipelineStateDesc> Ether::Graphics::Dx12Device::CreatePipelineStateDesc() const
{
    return std::make_unique<Dx12PipelineStateDesc>();
}

std::unique_ptr<Ether::Graphics::RhiRootSignature> Ether::Graphics::Dx12Device::CreateRootSignature(
    const RhiRootSignatureDesc& desc) const
{
    std::unique_ptr<Dx12RootSignature> dx12Obj = std::make_unique<Dx12RootSignature>();
    wrl::ComPtr<ID3DBlob> rsBlob, errBlob;
    const Dx12RootSignatureDesc& dx12Desc = dynamic_cast<const Dx12RootSignatureDesc&>(desc);

    HRESULT hr = D3D12SerializeRootSignature(
        &dx12Desc.m_Dx12RootSignatureDesc,
        D3D_ROOT_SIGNATURE_VERSION_1,
        rsBlob.GetAddressOf(),
        errBlob.GetAddressOf());

    if (FAILED(hr))
        LogGraphicsError("Failed to serialize root signature");

    hr = m_Device->CreateRootSignature(
        1,
        rsBlob->GetBufferPointer(),
        rsBlob->GetBufferSize(),
        IID_PPV_ARGS(&dx12Obj->m_RootSignature));

    if (FAILED(hr))
        LogGraphicsError("Failed to create DX12 Root Signature");

    return dx12Obj;
}

std::unique_ptr<Ether::Graphics::RhiPipelineState> Ether::Graphics::Dx12Device::CreatePipelineState(
    const RhiPipelineStateDesc& desc) const
{
    std::unique_ptr<Dx12PipelineState> dx12Obj = std::make_unique<Dx12PipelineState>(desc);
    const Dx12PipelineStateDesc& dx12Desc = dynamic_cast<const Dx12PipelineStateDesc&>(desc);

    HRESULT hr = m_Device->CreateGraphicsPipelineState(
        &dx12Desc.m_Dx12PsoDesc,
        IID_PPV_ARGS(&dx12Obj->m_PipelineState));

    if (FAILED(hr))
        LogGraphicsError("Failed to create DX12 Pipeline State Object");

    return dx12Obj;
}

std::unique_ptr<Ether::Graphics::RhiCommandAllocator> Ether::Graphics::Dx12Device::CreateCommandAllocator(
    RhiCommandAllocatorDesc desc) const
{
    std::unique_ptr<Dx12CommandAllocator> dx12View = std::make_unique<Dx12CommandAllocator>(desc.m_Type);

    HRESULT hr = m_Device->CreateCommandAllocator(Translate(desc.m_Type), IID_PPV_ARGS(&dx12View->m_Allocator));

    if (FAILED(hr))
        LogGraphicsError("Failed to create DX12 Command Allocator");

    return dx12View;
}

std::unique_ptr<Ether::Graphics::RhiCommandList> Ether::Graphics::Dx12Device::CreateCommandList(
    RhiCommandListDesc desc) const
{
    std::unique_ptr<Dx12CommandList> dx12Obj = std::make_unique<Dx12CommandList>(desc.m_Type);
    Dx12CommandAllocator* allocator = dynamic_cast<Dx12CommandAllocator*>(desc.m_Allocator);

    HRESULT hr = m_Device->CreateCommandList(
        1,
        Translate(desc.m_Type),
        allocator->m_Allocator.Get(),
        nullptr,
        IID_PPV_ARGS(&dx12Obj->m_CommandList));

    if (FAILED(hr))
        LogGraphicsError("Failed to create DX12 Command List");

    dx12Obj->m_CommandList->SetName(ToWideString(desc.m_Name).c_str());
    return dx12Obj;
}

std::unique_ptr<Ether::Graphics::RhiCommandQueue> Ether::Graphics::Dx12Device::CreateCommandQueue(
    RhiCommandQueueDesc desc) const
{
    std::unique_ptr<Dx12CommandQueue> dx12Obj = std::make_unique<Dx12CommandQueue>(desc.m_Type);

    auto creationDesc = Translate(desc);
    HRESULT hr = m_Device->CreateCommandQueue(&creationDesc, IID_PPV_ARGS(&dx12Obj->m_CommandQueue));

    if (FAILED(hr))
        LogGraphicsError("Failed to create DX12 Command Queue");

    return dx12Obj;
}

std::unique_ptr<Ether::Graphics::RhiDescriptorHeap> Ether::Graphics::Dx12Device::CreateDescriptorHeap(
    RhiDescriptorHeapDesc desc) const
{
    std::unique_ptr<Dx12DescriptorHeap> dx12Obj = std::make_unique<Dx12DescriptorHeap>();

    auto creationDesc = Translate(desc);
    HRESULT hr = m_Device->CreateDescriptorHeap(&creationDesc, IID_PPV_ARGS(&dx12Obj->m_Heap));

    if (FAILED(hr))
        LogGraphicsError("Failed to create DX12 Descriptor Heap");

    dx12Obj->m_HandleIncrementSize = m_Device->GetDescriptorHandleIncrementSize(Translate(desc.m_Type));
    return dx12Obj;
}

std::unique_ptr<Ether::Graphics::RhiFence> Ether::Graphics::Dx12Device::CreateFence() const
{
    std::unique_ptr<Dx12Fence> dx12Obj = std::make_unique<Dx12Fence>();
    HRESULT hr = m_Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&dx12Obj->m_Fence));

    if (FAILED(hr))
        LogGraphicsError("Failed to create DX12 Fence");

    return dx12Obj;
}

std::unique_ptr<Ether::Graphics::RhiSwapChain> Ether::Graphics::Dx12Device::CreateSwapChain(RhiSwapChainDesc desc) const
{
    std::unique_ptr<Dx12SwapChain> dx12Obj = std::make_unique<Dx12SwapChain>();

    const auto dx12CommandQueue = dynamic_cast<Dx12CommandQueue*>(desc.m_CommandQueue);

    wrl::ComPtr<IDXGIFactory4> dxgiFactory = dynamic_cast<Dx12Module&>(GraphicCore::GetModule()).m_DxgiFactory;
    wrl::ComPtr<IDXGISwapChain1> swapChain1;
    auto creationDesc = Translate(desc);
    HRESULT hr = dxgiFactory->CreateSwapChainForHwnd(
        dx12CommandQueue->m_CommandQueue.Get(),
        (HWND)desc.m_SurfaceTarget,
        &creationDesc,
        nullptr,
        nullptr,
        &swapChain1);
    if (FAILED(hr))
        LogGraphicsFatal("Failed to create DX12 SwapChain");

    hr = dxgiFactory->MakeWindowAssociation((HWND)desc.m_SurfaceTarget, DXGI_MWA_NO_ALT_ENTER);
    if (FAILED(hr))
        LogGraphicsWarning("Failed to disable alt-enter for hwnd");

    swapChain1.As(&dx12Obj->m_SwapChain);
    return dx12Obj;
}

std::unique_ptr<Ether::Graphics::RhiRenderTargetView> Ether::Graphics::Dx12Device::CreateRenderTargetView(
    RhiRenderTargetViewDesc desc) const
{
    std::unique_ptr<Dx12RenderTargetView> dx12Obj = std::make_unique<Dx12RenderTargetView>();

    dx12Obj->m_CpuAddress = desc.m_TargetCpuAddr;
    dx12Obj->m_Format = desc.m_Format;
    const auto d3dResource = dynamic_cast<Dx12Resource*>(desc.m_Resource);

    auto creationDesc = Translate(desc);
    m_Device->CreateRenderTargetView(d3dResource->m_Resource.Get(), &creationDesc, { dx12Obj->m_CpuAddress });

    return dx12Obj;
}

std::unique_ptr<Ether::Graphics::RhiDepthStencilView> Ether::Graphics::Dx12Device::CreateDepthStencilView(
    RhiDepthStencilViewDesc desc) const
{
    std::unique_ptr<Dx12DepthStencilView> dx12Obj = std::make_unique<Dx12DepthStencilView>();

    dx12Obj->m_CpuAddress = desc.m_TargetCpuAddr;
    dx12Obj->m_Format = desc.m_Format;
    const auto d3dResource = dynamic_cast<Dx12Resource*>(desc.m_Resource);

    auto creationDesc = Translate(desc);
    m_Device->CreateDepthStencilView(d3dResource->m_Resource.Get(), &creationDesc, { dx12Obj->m_CpuAddress });

    return dx12Obj;
}

std::unique_ptr<Ether::Graphics::RhiShaderResourceView> Ether::Graphics::Dx12Device::CreateShaderResourceView(
    RhiShaderResourceViewDesc desc) const
{
    std::unique_ptr<Dx12ShaderResourceView> dx12Obj = std::make_unique<Dx12ShaderResourceView>();

    dx12Obj->m_CpuAddress = desc.m_TargetCpuAddr;
    const auto d3dResource = dynamic_cast<Dx12Resource*>(desc.m_Resource);

    auto creationDesc = Translate(desc);
    m_Device->CreateShaderResourceView(d3dResource->m_Resource.Get(), &creationDesc, { dx12Obj->m_CpuAddress });

    return dx12Obj;
}

std::unique_ptr<Ether::Graphics::RhiConstantBufferView> Ether::Graphics::Dx12Device::CreateConstantBufferView(
    RhiConstantBufferViewDesc desc) const
{
    std::unique_ptr<Dx12ConstantBufferView> dx12Obj = std::make_unique<Dx12ConstantBufferView>();
    dx12Obj->m_CpuAddress = desc.m_TargetCpuAddr;

    auto creationDesc = Translate(desc);
    m_Device->CreateConstantBufferView(&creationDesc, { dx12Obj->m_CpuAddress });

    return dx12Obj;
}

std::unique_ptr<Ether::Graphics::RhiUnorderedAccessView> Ether::Graphics::Dx12Device::CreateUnorderedAccessView(
    RhiUnorderedAccessViewDesc desc) const
{
    throw std::runtime_error("Not yet implemented");
    // std::unique_ptr<Ether::Graphics::Dx12UnorderedAccessView> dx12View =
    // std::make_unique<Ether::Graphics::Dx12UnorderedAccessView>();

    // dx12View->m_CpuHandle = desc.m_TargetCpuHandle;
    // const auto d3dResource = dynamic_cast<Dx12Resource*>(desc.m_Resource);

    // auto creationDesc = Translate(desc);
    // m_Device->CreateUnorderedAccessView(
    //     d3dResource->m_Resource.Get(),
    //     &creationDesc,
    //     Translate(dx12View->m_CpuHandle)
    //);

    // return dx12View;
}

std::unique_ptr<Ether::Graphics::RhiResource> Ether::Graphics::Dx12Device::CreateCommittedResource(
    RhiCommitedResourceDesc desc) const
{
    std::unique_ptr<Dx12Resource> dx12Obj = std::make_unique<Dx12Resource>(desc.m_Name);

    auto creationDesc = Translate(desc.m_ResourceDesc);
    auto heapDesc = CD3DX12_HEAP_PROPERTIES(Translate(desc.m_HeapType));

    D3D12_CLEAR_VALUE clearValue;
    if (desc.m_ClearValue != nullptr)
        clearValue = Translate(*desc.m_ClearValue);

    HRESULT hr = m_Device->CreateCommittedResource(
        &heapDesc,
        D3D12_HEAP_FLAG_NONE,
        &creationDesc,
        Translate(desc.m_State),
        desc.m_ClearValue == nullptr ? nullptr : &clearValue,
        IID_PPV_ARGS(&dx12Obj->m_Resource));

    if (FAILED(hr))
        LogGraphicsError("Failed to create DX12 commited resource (%s)", desc.m_Name.c_str());

    dx12Obj->m_Resource->SetName(ToWideString(desc.m_Name).c_str());
    return dx12Obj;
}

std::unique_ptr<Ether::Graphics::RhiShader> Ether::Graphics::Dx12Device::CreateShader(RhiShaderDesc desc) const
{
    std::unique_ptr<Dx12Shader> dx12Obj = std::make_unique<Dx12Shader>(desc);
    return dx12Obj;
}

#endif // ETH_GRAPHICS_DX12
