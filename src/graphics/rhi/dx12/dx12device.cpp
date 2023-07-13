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

#include "graphics/rhi/dx12/dx12accelerationstructure.h"
#include "graphics/rhi/dx12/dx12commandallocator.h"
#include "graphics/rhi/dx12/dx12commandlist.h"
#include "graphics/rhi/dx12/dx12commandqueue.h"
#include "graphics/rhi/dx12/dx12descriptorheap.h"
#include "graphics/rhi/dx12/dx12fence.h"
#include "graphics/rhi/dx12/dx12graphicpipelinestate.h"
#include "graphics/rhi/dx12/dx12computepipelinestate.h"
#include "graphics/rhi/dx12/dx12resource.h"
#include "graphics/rhi/dx12/dx12rootsignature.h"
#include "graphics/rhi/dx12/dx12swapchain.h"
#include "graphics/rhi/dx12/dx12shader.h"
#include "graphics/rhi/dx12/dx12raytracingpipelinestate.h"
#include "graphics/rhi/dx12/dx12raytracingshaderbindingtable.h"
#include "graphics/rhi/dx12/dx12translation.h"

#include "graphics/resources/mesh.h"

#ifdef ETH_GRAPHICS_DX12

std::unique_ptr<Ether::Graphics::RhiCommandAllocator> Ether::Graphics::Dx12Device::CreateCommandAllocator(
    const RhiCommandType& type) const
{
    std::unique_ptr<Dx12CommandAllocator> dx12View = std::make_unique<Dx12CommandAllocator>(type);

    HRESULT hr = m_Device->CreateCommandAllocator(Translate(type), IID_PPV_ARGS(&dx12View->m_Allocator));

    if (FAILED(hr))
        LogGraphicsFatal("Failed to create DirectX12 Command Allocator");

    return dx12View;
}

std::unique_ptr<Ether::Graphics::RhiCommandList> Ether::Graphics::Dx12Device::CreateCommandList(
    const char* name,
    const RhiCommandType& type) const
{
    std::unique_ptr<Dx12CommandList> dx12Obj = std::make_unique<Dx12CommandList>(type);

    Dx12CommandAllocator* allocator = dynamic_cast<Dx12CommandAllocator*>(
        &GraphicCore::GetCommandManager().GetAllocatorPool(type).RequestAllocator());

    HRESULT hr = m_Device->CreateCommandList(
        1,
        Translate(type),
        allocator->m_Allocator.Get(),
        nullptr,
        IID_PPV_ARGS(&dx12Obj->m_CommandList));

    if (FAILED(hr))
        LogGraphicsFatal("Failed to create DirectX12 Command List");

    GraphicCore::GetCommandManager().GetAllocatorPool(type).DiscardAllocator(*allocator);

    dx12Obj->m_CommandList->SetName(ToWideString(name).c_str());
    return dx12Obj;
}

std::unique_ptr<Ether::Graphics::RhiCommandQueue> Ether::Graphics::Dx12Device::CreateCommandQueue(
    const RhiCommandType& type) const
{
    std::unique_ptr<Dx12CommandQueue> dx12Obj = std::make_unique<Dx12CommandQueue>(type);

    D3D12_COMMAND_QUEUE_DESC dx12Desc = {};
    dx12Desc.Type = Translate(type);
    dx12Desc.NodeMask = 0;
    dx12Desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    dx12Desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

    HRESULT hr = m_Device->CreateCommandQueue(&dx12Desc, IID_PPV_ARGS(&dx12Obj->m_CommandQueue));

    if (FAILED(hr))
        LogGraphicsFatal("Failed to create DirectX12 Command Queue");

    return dx12Obj;
}

std::unique_ptr<Ether::Graphics::RhiDescriptorHeap> Ether::Graphics::Dx12Device::CreateDescriptorHeap(
    const RhiDescriptorHeapType& type,
    uint32_t numDescriptors,
    bool isShaderVisible) const
{
    std::unique_ptr<Dx12DescriptorHeap> dx12Obj = std::make_unique<Dx12DescriptorHeap>();

    D3D12_DESCRIPTOR_HEAP_DESC dx12Desc = {};
    dx12Desc.Flags = isShaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    dx12Desc.Type = Translate(type);
    dx12Desc.NumDescriptors = numDescriptors;
    HRESULT hr = m_Device->CreateDescriptorHeap(&dx12Desc, IID_PPV_ARGS(&dx12Obj->m_Heap));

    if (FAILED(hr))
        LogGraphicsFatal("Failed to create DirectX12 Descriptor Heap");

    dx12Obj->m_HandleIncrementSize = m_Device->GetDescriptorHandleIncrementSize(Translate(type));
    return dx12Obj;
}

std::unique_ptr<Ether::Graphics::RhiFence> Ether::Graphics::Dx12Device::CreateFence() const
{
    std::unique_ptr<Dx12Fence> dx12Obj = std::make_unique<Dx12Fence>();
    HRESULT hr = m_Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&dx12Obj->m_Fence));

    if (FAILED(hr))
        LogGraphicsFatal("Failed to create DirectX12 Fence");

    return dx12Obj;
}

std::unique_ptr<Ether::Graphics::RhiSwapChain> Ether::Graphics::Dx12Device::CreateSwapChain(
    const RhiSwapChainDesc& desc) const
{
    std::unique_ptr<Dx12SwapChain> dx12Obj = std::make_unique<Dx12SwapChain>();

    const Dx12CommandQueue* cmdQueue = dynamic_cast<Dx12CommandQueue*>(
        &GraphicCore::GetCommandManager().GetGraphicQueue());

    wrl::ComPtr<IDXGIFactory4> dxgiFactory = dynamic_cast<Dx12Module&>(GraphicCore::GetModule()).m_DxgiFactory;
    wrl::ComPtr<IDXGISwapChain1> swapChain1;

    DXGI_SWAP_CHAIN_DESC1 dx12Desc = {};
    dx12Desc.Width = desc.m_Resolution.x;
    dx12Desc.Height = desc.m_Resolution.y;
    dx12Desc.Format = Translate(desc.m_Format);
    dx12Desc.Stereo = false;
    dx12Desc.SampleDesc = Translate(desc.m_SampleDesc);
    dx12Desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    dx12Desc.BufferCount = desc.m_NumBuffers;
    dx12Desc.Scaling = DXGI_SCALING_STRETCH;
    dx12Desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    dx12Desc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    dx12Desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

    HRESULT hr = dxgiFactory->CreateSwapChainForHwnd(
        cmdQueue->m_CommandQueue.Get(),
        (HWND)desc.m_SurfaceTarget,
        &dx12Desc,
        nullptr,
        nullptr,
        &swapChain1);
    if (FAILED(hr))
        LogGraphicsFatal("Failed to create DirectX12 SwapChain");

    hr = dxgiFactory->MakeWindowAssociation((HWND)desc.m_SurfaceTarget, DXGI_MWA_NO_ALT_ENTER);
    if (FAILED(hr))
        LogGraphicsWarning("Failed to disable alt-enter for hwnd");

    swapChain1.As(&dx12Obj->m_SwapChain);
    return dx12Obj;
}

std::unique_ptr<Ether::Graphics::RhiShader> Ether::Graphics::Dx12Device::CreateShader(const RhiShaderDesc& desc) const
{
    std::unique_ptr<Dx12Shader> dx12Obj = std::make_unique<Dx12Shader>(desc);
    return dx12Obj;
}

std::unique_ptr<Ether::Graphics::RhiRootSignatureDesc> Ether::Graphics::Dx12Device::CreateRootSignatureDesc(
    uint32_t numParams,
    uint32_t numSamplers,
    bool isLocal) const
{
    return std::make_unique<Dx12RootSignatureDesc>(numParams, numSamplers, isLocal);
}

std::unique_ptr<Ether::Graphics::RhiGraphicPipelineStateDesc> Ether::Graphics::Dx12Device::CreateGraphicPipelineStateDesc() const
{
    return std::make_unique<Dx12GraphicPipelineStateDesc>();
}

std::unique_ptr<Ether::Graphics::RhiComputePipelineStateDesc> Ether::Graphics::Dx12Device::CreateComputePipelineStateDesc() const
{
    return std::make_unique<Dx12ComputePipelineStateDesc>();
}

std::unique_ptr<Ether::Graphics::RhiRaytracingPipelineStateDesc> Ether::Graphics::Dx12Device::CreateRaytracingPipelineStateDesc() const
{
    return std::make_unique<Dx12RaytracingPipelineStateDesc>();
}

std::unique_ptr<Ether::Graphics::RhiResource> Ether::Graphics::Dx12Device::CreateRaytracingShaderBindingTable(
    const char* name,
    const RhiRaytracingShaderBindingTableDesc& desc) const
{
    uint32_t entrySize = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
    entrySize += desc.m_MaxRootSignatureSize;
    entrySize = AlignUp(entrySize, D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT);

    std::unique_ptr<Dx12RaytracingShaderBindingTable> dx12Obj = std::make_unique<Dx12RaytracingShaderBindingTable>(
        name,
        entrySize,
        3);

    RhiCommitedResourceDesc bufferDesc = {};
    bufferDesc.m_Name = "RaytracingShaderTable::ShaderTable";
    bufferDesc.m_HeapType = RhiHeapType::Upload;
    bufferDesc.m_State = RhiResourceState::GenericRead;
    bufferDesc.m_ResourceDesc = RhiCreateBufferResourceDesc(dx12Obj->GetTableSize());
    dx12Obj->m_Buffer = CreateCommittedResource(bufferDesc);

    uint8_t* mappedAddr;
    dx12Obj->m_Buffer->Map((void**)&mappedAddr);

    wrl::ComPtr<ID3D12StateObjectProperties> stateObjectProperties;
    ((Dx12RaytracingPipelineState*)(desc.m_RaytracingPipelineState))->m_PipelineState->QueryInterface(IID_PPV_ARGS(&stateObjectProperties));

    memcpy(
        mappedAddr,
        stateObjectProperties->GetShaderIdentifier(desc.m_RayGenShaderName),
        D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);

    memcpy(
        mappedAddr + 1 * entrySize,
        stateObjectProperties->GetShaderIdentifier(desc.m_MissShaderName),
        D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);

    memcpy(
        mappedAddr + 2 * entrySize,
        stateObjectProperties->GetShaderIdentifier(desc.m_HitGroupName),
        D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);

    dx12Obj->m_Buffer->Unmap();
    return dx12Obj;
}

std::unique_ptr<Ether::Graphics::RhiAccelerationStructure> Ether::Graphics::Dx12Device::CreateAccelerationStructure(
    const RhiTopLevelAccelerationStructureDesc& desc) const
{
    std::unique_ptr<Dx12AccelerationStructure> dx12Obj = std::make_unique<Dx12AccelerationStructure>();
    Visual* visuals = reinterpret_cast<Visual*>(desc.m_Visuals);

    dx12Obj->m_Inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
    dx12Obj->m_Inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_BUILD;
    dx12Obj->m_Inputs.NumDescs = desc.m_NumVisuals;
    dx12Obj->m_Inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;

    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO info;
    m_Device->GetRaytracingAccelerationStructurePrebuildInfo(&dx12Obj->m_Inputs, &info);
    dx12Obj->m_Size = info.ResultDataMaxSizeInBytes;

    RhiCommitedResourceDesc scratchBufferDesc = {};
    scratchBufferDesc.m_Name = "TLAS::ScratchBuffer";
    scratchBufferDesc.m_HeapType = RhiHeapType::Default;
    scratchBufferDesc.m_State = RhiResourceState::Common;
    scratchBufferDesc.m_ResourceDesc = RhiCreateBufferResourceDesc(info.ScratchDataSizeInBytes);
    scratchBufferDesc.m_ResourceDesc.m_Flag = RhiResourceFlag::AllowUnorderedAccess;
    dx12Obj->m_ScratchBuffer = CreateCommittedResource(scratchBufferDesc);

    RhiCommitedResourceDesc dataBufferDesc = {};
    dataBufferDesc.m_Name = "TLAS::DataBuffer";
    dataBufferDesc.m_HeapType = RhiHeapType::Default;
    dataBufferDesc.m_State = RhiResourceState::AccelerationStructure;
    dataBufferDesc.m_ResourceDesc = RhiCreateBufferResourceDesc(info.ResultDataMaxSizeInBytes);
    dataBufferDesc.m_ResourceDesc.m_Flag = RhiResourceFlag::AllowUnorderedAccess;
    dx12Obj->m_DataBuffer = CreateCommittedResource(dataBufferDesc);

    if (desc.m_NumVisuals > 0)
    {
        RhiCommitedResourceDesc instanceBufferDesc = {};
        instanceBufferDesc.m_Name = "TLAS::InstanceBuffer";
        instanceBufferDesc.m_HeapType = RhiHeapType::Upload;
        instanceBufferDesc.m_State = RhiResourceState::GenericRead;
        instanceBufferDesc.m_ResourceDesc = RhiCreateBufferResourceDesc(
            sizeof(D3D12_RAYTRACING_INSTANCE_DESC) * desc.m_NumVisuals);
        dx12Obj->m_InstanceDescBuffer = CreateCommittedResource(instanceBufferDesc);

        D3D12_RAYTRACING_INSTANCE_DESC* instanceDesc;
        dx12Obj->m_InstanceDescBuffer->Map(reinterpret_cast<void**>(&instanceDesc));
        for (uint64_t i = 0; i < desc.m_NumVisuals; ++i, ++instanceDesc)
        {
            instanceDesc->InstanceID = i;
            instanceDesc->InstanceContributionToHitGroupIndex = 0;
            instanceDesc->Flags = D3D12_RAYTRACING_INSTANCE_FLAG_NONE;
            instanceDesc->InstanceMask = 0xFF;
            ethMatrix4x4 I; // Identity matrix
            memcpy(instanceDesc->Transform, &I, sizeof(instanceDesc->Transform));
            instanceDesc
                ->AccelerationStructure = visuals[i].m_Mesh->GetAccelerationStructure().m_DataBuffer->GetGpuAddress();
        }
        dx12Obj->m_InstanceDescBuffer->Unmap();
    }

    return dx12Obj;
}

std::unique_ptr<Ether::Graphics::RhiAccelerationStructure> Ether::Graphics::Dx12Device::CreateAccelerationStructure(
    const RhiBottomLevelAccelerationStructureDesc& desc) const
{
    std::unique_ptr<Dx12AccelerationStructure> dx12Obj = std::make_unique<Dx12AccelerationStructure>();

    AssertGraphics(
        desc.m_NumMeshes == 1,
        "Only single mesh BLAS is supported for now. NumMeshes: %u",
        desc.m_NumMeshes);

    Mesh* mesh = (reinterpret_cast<Mesh**>(desc.m_Meshes))[0];

    dx12Obj->m_GeometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
    dx12Obj->m_GeometryDesc.Triangles.VertexBuffer.StartAddress = mesh->GetVertexBufferView().m_TargetGpuAddress;
    dx12Obj->m_GeometryDesc.Triangles.VertexBuffer.StrideInBytes = mesh->GetVertexBufferView().m_Stride;
    dx12Obj->m_GeometryDesc.Triangles.VertexFormat = Translate(Mesh::s_VertexBufferPositionFormat);
    dx12Obj->m_GeometryDesc.Triangles.VertexCount = mesh->GetNumVertices();
    dx12Obj->m_GeometryDesc.Triangles.IndexBuffer = mesh->GetIndexBufferView().m_TargetGpuAddress;
    dx12Obj->m_GeometryDesc.Triangles.IndexCount = mesh->GetNumIndices();
    dx12Obj->m_GeometryDesc.Triangles.IndexFormat = Translate(Mesh::s_IndexBufferFormat);
    dx12Obj->m_GeometryDesc.Flags = desc.m_IsOpaque ? D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE
                                                    : D3D12_RAYTRACING_GEOMETRY_FLAG_NONE;

    dx12Obj->m_Inputs = {};
    dx12Obj->m_Inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
    dx12Obj->m_Inputs.NumDescs = 1;
    dx12Obj->m_Inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
    dx12Obj->m_Inputs.pGeometryDescs = &dx12Obj->m_GeometryDesc;
    if (desc.m_IsStatic)
        dx12Obj->m_Inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
    else
        dx12Obj->m_Inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_BUILD;

    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO info = {};
    m_Device->GetRaytracingAccelerationStructurePrebuildInfo(&dx12Obj->m_Inputs, &info);
    dx12Obj->m_Size = info.ResultDataMaxSizeInBytes;

    RhiCommitedResourceDesc scratchBufferDesc = {};
    scratchBufferDesc.m_Name = "BLAS::ScratchBuffer";
    scratchBufferDesc.m_HeapType = RhiHeapType::Default;
    scratchBufferDesc.m_State = RhiResourceState::Common;
    scratchBufferDesc.m_ResourceDesc = RhiCreateBufferResourceDesc(info.ScratchDataSizeInBytes);
    scratchBufferDesc.m_ResourceDesc.m_Flag = RhiResourceFlag::AllowUnorderedAccess;
    dx12Obj->m_ScratchBuffer = CreateCommittedResource(scratchBufferDesc);

    RhiCommitedResourceDesc dataBufferDesc = {};
    dataBufferDesc.m_Name = "BLAS::DataBuffer";
    dataBufferDesc.m_HeapType = RhiHeapType::Default;
    dataBufferDesc.m_State = RhiResourceState::AccelerationStructure;
    dataBufferDesc.m_ResourceDesc = RhiCreateBufferResourceDesc(info.ResultDataMaxSizeInBytes);
    dataBufferDesc.m_ResourceDesc.m_Flag = RhiResourceFlag::AllowUnorderedAccess;
    dx12Obj->m_DataBuffer = CreateCommittedResource(dataBufferDesc);

    return dx12Obj;
}

std::unique_ptr<Ether::Graphics::RhiResource> Ether::Graphics::Dx12Device::CreateCommittedResource(
    const RhiCommitedResourceDesc& desc) const
{
    std::unique_ptr<Dx12Resource> dx12Obj = std::make_unique<Dx12Resource>(desc.m_Name);

    auto creationDesc = Translate(desc.m_ResourceDesc);
    auto heapDesc = CD3DX12_HEAP_PROPERTIES(Translate(desc.m_HeapType));

    D3D12_CLEAR_VALUE clearValue = Translate(desc.m_ClearValue);
    bool haveClearColor = true;
    haveClearColor &= creationDesc.Dimension != D3D12_RESOURCE_DIMENSION_BUFFER;
    haveClearColor &=
        (creationDesc.Flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET ||
         creationDesc.Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);

    HRESULT hr = m_Device->CreateCommittedResource(
        &heapDesc,
        D3D12_HEAP_FLAG_NONE,
        &creationDesc,
        Translate(desc.m_State),
        haveClearColor ? &clearValue : nullptr,
        IID_PPV_ARGS(&dx12Obj->m_Resource));

    if (FAILED(hr))
        LogGraphicsFatal("Failed to create DirectX12 commited resource (%s)", desc.m_Name);

    dx12Obj->m_Resource->SetName(ToWideString(desc.m_Name).c_str());
    dx12Obj->SetNumMips(desc.m_ResourceDesc.m_MipLevels);
    dx12Obj->SetState(desc.m_State);
    return dx12Obj;
}

std::unique_ptr<Ether::Graphics::RhiRootSignature> Ether::Graphics::Dx12Device::CreateRootSignature(
    const char* name,
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
        LogGraphicsFatal("Failed to serialize root signature: %s", errBlob->GetBufferPointer());

    hr = m_Device->CreateRootSignature(
        1,
        rsBlob->GetBufferPointer(),
        rsBlob->GetBufferSize(),
        IID_PPV_ARGS(&dx12Obj->m_RootSignature));

    if (FAILED(hr))
        LogGraphicsFatal("Failed to create DirectX12 Root Signature");

    dx12Obj->m_RootSignature->SetName(ToWideString(name).c_str());

    return dx12Obj;
}

void Ether::Graphics::Dx12Device::CopyDescriptors(
    uint32_t numDescriptors,
    RhiCpuAddress srcAddr,
    RhiCpuAddress destAddr,
    RhiDescriptorHeapType type) const
{
    m_Device->CopyDescriptorsSimple(numDescriptors, { destAddr }, { srcAddr }, Translate(type));
}

void Ether::Graphics::Dx12Device::CopySampler(const RhiSamplerParameterDesc& sampler, RhiCpuAddress destAddr) const
{
    D3D12_SAMPLER_DESC dx12Desc = {};
    dx12Desc.Filter = Translate(sampler.m_Filter);
    dx12Desc.AddressU = Translate(sampler.m_AddressU);
    dx12Desc.AddressV = Translate(sampler.m_AddressV);
    dx12Desc.AddressW = Translate(sampler.m_AddressW);
    dx12Desc.MipLODBias = sampler.m_MipLodBias;
    dx12Desc.MaxAnisotropy = sampler.m_MaxAnisotropy;
    dx12Desc.ComparisonFunc = Translate(sampler.m_ComparisonFunc);
    dx12Desc.MinLOD = sampler.m_MinLod;
    dx12Desc.MaxLOD = sampler.m_MaxLod;

    switch (sampler.m_BorderColor)
    {
    case RhiBorderColor::OpaqueBlack:
        dx12Desc.BorderColor[0] = 0;
        dx12Desc.BorderColor[1] = 0;
        dx12Desc.BorderColor[2] = 0;
        dx12Desc.BorderColor[3] = 1;
        break;
    case RhiBorderColor::OpaqueWhite:
        dx12Desc.BorderColor[0] = 1;
        dx12Desc.BorderColor[1] = 1;
        dx12Desc.BorderColor[2] = 1;
        dx12Desc.BorderColor[3] = 1;
        break;
    case RhiBorderColor::TransparentBlack:
        dx12Desc.BorderColor[0] = 0;
        dx12Desc.BorderColor[1] = 0;
        dx12Desc.BorderColor[2] = 0;
        dx12Desc.BorderColor[3] = 0;
        break;
    }

    m_Device->CreateSampler(&dx12Desc, { destAddr });
}

void Ether::Graphics::Dx12Device::InitializeRenderTargetView(RhiRenderTargetView& rtv, const RhiResource& resource) const
{
    const auto d3dResource = dynamic_cast<const Dx12Resource*>(&resource);

    D3D12_RENDER_TARGET_VIEW_DESC desc = {};
    desc.Format = Translate(rtv.GetFormat());
    switch (rtv.GetDimension())
    {
    case RhiResourceDimension::Texture2D:
        desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
        desc.Texture2D.MipSlice = 0;
        break;
    default:
        LogGraphicsFatal("Not yet supported");
    }

    m_Device->CreateRenderTargetView(d3dResource->m_Resource.Get(), &desc, { rtv.GetCpuAddress() });
}

void Ether::Graphics::Dx12Device::InitializeDepthStencilView(RhiDepthStencilView& dsv, const RhiResource& resource) const
{
    const auto d3dResource = dynamic_cast<const Dx12Resource*>(&resource);

    D3D12_DEPTH_STENCIL_VIEW_DESC desc = {};
    desc.Format = Translate(dsv.GetFormat());
    desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    desc.Texture2D.MipSlice = 0;
    desc.Flags = D3D12_DSV_FLAG_NONE;

    m_Device->CreateDepthStencilView(d3dResource->m_Resource.Get(), &desc, { dsv.GetCpuAddress() });
}

void Ether::Graphics::Dx12Device::InitializeShaderResourceView(RhiShaderResourceView& srv, const RhiResource& resource) const
{
    const auto d3dResource = dynamic_cast<const Dx12Resource*>(&resource);
    ID3D12Resource* id3d12Resource = d3dResource->m_Resource.Get();

    D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
    desc.Format = Translate(srv.GetFormat());
    desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

    switch (srv.GetDimension())
    {
    case RhiResourceDimension::Texture2D:
        desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        desc.Texture2D.MipLevels = resource.GetNumMips();
        desc.Texture2D.MostDetailedMip = 0;
        desc.Texture2D.PlaneSlice = 0;
        desc.Texture2D.ResourceMinLODClamp = 0;
        break;
    case RhiResourceDimension::TextureCube:
        desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
        desc.Texture2D.MipLevels = resource.GetNumMips();
        desc.Texture2D.MostDetailedMip = 0;
        desc.Texture2D.PlaneSlice = 0;
        desc.Texture2D.ResourceMinLODClamp = 0;
        break;
    case RhiResourceDimension::RTAccelerationStructure:
        desc.ViewDimension = D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE;
        desc.RaytracingAccelerationStructure.Location = d3dResource->GetGpuAddress();
        id3d12Resource = nullptr;
        break;
    case RhiResourceDimension::Buffer:
        desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
        desc.Buffer.FirstElement = 0;
        desc.Buffer.NumElements = 1;
        desc.Buffer.StructureByteStride = 0;
        break;
    case RhiResourceDimension::StructuredBuffer:
        desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
        desc.Buffer.FirstElement = 0;
        desc.Buffer.NumElements = srv.GetStructuredBufferCount();
        desc.Buffer.StructureByteStride = srv.GetStructuredBufferStride();
        break;
    default:
        LogGraphicsFatal("Not yet supported");
    }

    m_Device->CreateShaderResourceView(id3d12Resource, &desc, { srv.GetCpuAddress() });
}

void Ether::Graphics::Dx12Device::InitializeUnorderedAccessView(RhiUnorderedAccessView& uav, const RhiResource& resource) const
{
    const auto d3dResource = dynamic_cast<const Dx12Resource*>(&resource);

    D3D12_UNORDERED_ACCESS_VIEW_DESC desc = {};
    desc.Format = Translate(uav.GetFormat());

    switch (uav.GetDimension())
    {
    case RhiResourceDimension::Texture2D:
        desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
        desc.Texture2D.MipSlice = 0;
        break;
    case RhiResourceDimension::Buffer:
        desc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
        desc.Buffer.FirstElement = 0;
        desc.Buffer.NumElements = 1;
        desc.Buffer.StructureByteStride = 0;
        break;
    case RhiResourceDimension::StructuredBuffer:
        desc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
        desc.Buffer.FirstElement = 0;
        desc.Buffer.NumElements = uav.GetStructuredBufferCount();
        desc.Buffer.StructureByteStride = uav.GetStructuredBufferStride();
        break;
    default:
        LogGraphicsFatal("Not yet supported");
    }

    m_Device->CreateUnorderedAccessView(d3dResource->m_Resource.Get(), nullptr, &desc, { uav.GetCpuAddress() });
}

void Ether::Graphics::Dx12Device::InitializeConstantBufferView(RhiConstantBufferView& cbv, const RhiResource& resource) const
{
    const auto d3dResource = dynamic_cast<const Dx12Resource*>(&resource);

    D3D12_CONSTANT_BUFFER_VIEW_DESC desc = {};
    desc.SizeInBytes = cbv.GetWidth();
    desc.BufferLocation = resource.GetGpuAddress();

    m_Device->CreateConstantBufferView(&desc, { cbv.GetCpuAddress() });
}

std::unique_ptr<Ether::Graphics::RhiPipelineState> Ether::Graphics::Dx12Device::CreateGraphicPipelineState(
    const char* name,
    const RhiGraphicPipelineStateDesc& desc) const
{
    std::unique_ptr<Dx12GraphicPipelineState> dx12Obj = std::make_unique<Dx12GraphicPipelineState>(desc);
    const Dx12GraphicPipelineStateDesc& dx12Desc = dynamic_cast<const Dx12GraphicPipelineStateDesc&>(desc);

    HRESULT hr = m_Device->CreateGraphicsPipelineState(
        &dx12Desc.m_Dx12PsoDesc,
        IID_PPV_ARGS(&dx12Obj->m_PipelineState));

    if (FAILED(hr))
        LogGraphicsFatal("Failed to create DirectX12 Pipeline State Object");

    dx12Obj->m_PipelineState->SetName(ToWideString(name).c_str());
    return dx12Obj;
}

std::unique_ptr<Ether::Graphics::RhiPipelineState> Ether::Graphics::Dx12Device::CreateComputePipelineState(
    const char* name,
    const RhiComputePipelineStateDesc& desc) const
{
    std::unique_ptr<Dx12ComputePipelineState> dx12Obj = std::make_unique<Dx12ComputePipelineState>(desc);
    const Dx12ComputePipelineStateDesc& dx12Desc = dynamic_cast<const Dx12ComputePipelineStateDesc&>(desc);

    HRESULT hr = m_Device->CreateComputePipelineState(
        &dx12Desc.m_Dx12PsoDesc,
        IID_PPV_ARGS(&dx12Obj->m_PipelineState));

    if (FAILED(hr))
        LogGraphicsFatal("Failed to create DirectX12 Pipeline State Object");

    dx12Obj->m_PipelineState->SetName(ToWideString(name).c_str());
    return dx12Obj;
}

std::unique_ptr<Ether::Graphics::RhiPipelineState> Ether::Graphics::Dx12Device::CreateRaytracingPipelineState(
    const char* name,
    const RhiRaytracingPipelineStateDesc& desc) const
{
    std::unique_ptr<Dx12RaytracingPipelineState> dx12Obj = std::make_unique<Dx12RaytracingPipelineState>(desc);
    const Dx12RaytracingPipelineStateDesc& dx12Desc = dynamic_cast<const Dx12RaytracingPipelineStateDesc&>(desc);

    D3D12_STATE_OBJECT_DESC stateObjDesc = {};
    stateObjDesc.NumSubobjects = dx12Desc.m_NumSubObjects;
    stateObjDesc.pSubobjects = dx12Desc.m_SubObjects;
    stateObjDesc.Type = D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE;

    HRESULT hr = m_Device->CreateStateObject(&stateObjDesc, IID_PPV_ARGS(&dx12Obj->m_PipelineState));
    if (FAILED(hr))
        LogGraphicsFatal("Failed to create DirectX12 Raytracing Pipeline State");

    return dx12Obj;
}

#endif // ETH_GRAPHICS_DX12
