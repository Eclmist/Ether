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
#include "graphics/rhi/dx12/dx12pipelinestate.h"
#include "graphics/rhi/dx12/dx12resource.h"
#include "graphics/rhi/dx12/dx12resourceviews.h"
#include "graphics/rhi/dx12/dx12rootsignature.h"
#include "graphics/rhi/dx12/dx12swapchain.h"
#include "graphics/rhi/dx12/dx12shader.h"
#include "graphics/rhi/dx12/dx12raytracingpipelinestate.h"
#include "graphics/rhi/dx12/dx12raytracingshaderbindingtable.h"
#include "graphics/rhi/dx12/dx12translation.h"

#include "graphics/resources/mesh.h"

#ifdef ETH_GRAPHICS_DX12

std::unique_ptr<Ether::Graphics::RhiCommandAllocator> Ether::Graphics::Dx12Device::CreateCommandAllocator(
    const RhiCommandAllocatorDesc& desc) const
{
    std::unique_ptr<Dx12CommandAllocator> dx12View = std::make_unique<Dx12CommandAllocator>(desc.m_Type);

    HRESULT hr = m_Device->CreateCommandAllocator(Translate(desc.m_Type), IID_PPV_ARGS(&dx12View->m_Allocator));

    if (FAILED(hr))
        LogGraphicsFatal("Failed to create DirectX12 Command Allocator");

    return dx12View;
}

std::unique_ptr<Ether::Graphics::RhiCommandList> Ether::Graphics::Dx12Device::CreateCommandList(
    const RhiCommandListDesc& desc) const
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
        LogGraphicsFatal("Failed to create DirectX12 Command List");

    dx12Obj->m_CommandList->SetName(ToWideString(desc.m_Name).c_str());
    return dx12Obj;
}

std::unique_ptr<Ether::Graphics::RhiCommandQueue> Ether::Graphics::Dx12Device::CreateCommandQueue(
    const RhiCommandQueueDesc& desc) const
{
    std::unique_ptr<Dx12CommandQueue> dx12Obj = std::make_unique<Dx12CommandQueue>(desc.m_Type);

    auto creationDesc = Translate(desc);
    HRESULT hr = m_Device->CreateCommandQueue(&creationDesc, IID_PPV_ARGS(&dx12Obj->m_CommandQueue));

    if (FAILED(hr))
        LogGraphicsFatal("Failed to create DirectX12 Command Queue");

    return dx12Obj;
}

std::unique_ptr<Ether::Graphics::RhiDescriptorHeap> Ether::Graphics::Dx12Device::CreateDescriptorHeap(
    const RhiDescriptorHeapDesc& desc) const
{
    std::unique_ptr<Dx12DescriptorHeap> dx12Obj = std::make_unique<Dx12DescriptorHeap>();

    auto creationDesc = Translate(desc);
    HRESULT hr = m_Device->CreateDescriptorHeap(&creationDesc, IID_PPV_ARGS(&dx12Obj->m_Heap));

    if (FAILED(hr))
        LogGraphicsFatal("Failed to create DirectX12 Descriptor Heap");

    dx12Obj->m_HandleIncrementSize = m_Device->GetDescriptorHandleIncrementSize(Translate(desc.m_Type));
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
        LogGraphicsFatal("Failed to create DirectX12 SwapChain");

    hr = dxgiFactory->MakeWindowAssociation((HWND)desc.m_SurfaceTarget, DXGI_MWA_NO_ALT_ENTER);
    if (FAILED(hr))
        LogGraphicsWarning("Failed to disable alt-enter for hwnd");

    swapChain1.As(&dx12Obj->m_SwapChain);
    return dx12Obj;
}

std::unique_ptr<Ether::Graphics::RhiRenderTargetView> Ether::Graphics::Dx12Device::CreateRenderTargetView(
    const RhiRenderTargetViewDesc& desc) const
{
    std::unique_ptr<Dx12RenderTargetView> dx12Obj = std::make_unique<Dx12RenderTargetView>();

    dx12Obj->m_CpuAddress = desc.m_TargetCpuAddress;
    dx12Obj->m_Format = desc.m_Format;
    const auto d3dResource = dynamic_cast<Dx12Resource*>(desc.m_Resource);

    auto creationDesc = Translate(desc);
    m_Device->CreateRenderTargetView(d3dResource->m_Resource.Get(), &creationDesc, { dx12Obj->m_CpuAddress });

    return dx12Obj;
}

std::unique_ptr<Ether::Graphics::RhiDepthStencilView> Ether::Graphics::Dx12Device::CreateDepthStencilView(
    const RhiDepthStencilViewDesc& desc) const
{
    std::unique_ptr<Dx12DepthStencilView> dx12Obj = std::make_unique<Dx12DepthStencilView>();

    dx12Obj->m_CpuAddress = desc.m_TargetCpuAddress;
    dx12Obj->m_Format = desc.m_Format;
    const auto d3dResource = dynamic_cast<Dx12Resource*>(desc.m_Resource);

    auto creationDesc = Translate(desc);
    m_Device->CreateDepthStencilView(d3dResource->m_Resource.Get(), &creationDesc, { dx12Obj->m_CpuAddress });

    return dx12Obj;
}

std::unique_ptr<Ether::Graphics::RhiShaderResourceView> Ether::Graphics::Dx12Device::CreateShaderResourceView(
    const RhiShaderResourceViewDesc& desc) const
{
    std::unique_ptr<Dx12ShaderResourceView> dx12Obj = std::make_unique<Dx12ShaderResourceView>();

    dx12Obj->m_CpuAddress = desc.m_TargetCpuAddress;
    const auto d3dResource = dynamic_cast<Dx12Resource*>(desc.m_Resource);

    auto creationDesc = Translate(desc);

    if (desc.m_Dimensions == RhiShaderResourceDimensions::RTAccelerationStructure)
        m_Device->CreateShaderResourceView(nullptr, &creationDesc, { dx12Obj->m_CpuAddress });
    else
        m_Device->CreateShaderResourceView(d3dResource->m_Resource.Get(), &creationDesc, { dx12Obj->m_CpuAddress });

    return dx12Obj;
}

std::unique_ptr<Ether::Graphics::RhiConstantBufferView> Ether::Graphics::Dx12Device::CreateConstantBufferView(
    const RhiConstantBufferViewDesc& desc) const
{
    std::unique_ptr<Dx12ConstantBufferView> dx12Obj = std::make_unique<Dx12ConstantBufferView>();
    dx12Obj->m_CpuAddress = desc.m_TargetCpuAddress;

    auto creationDesc = Translate(desc);
    m_Device->CreateConstantBufferView(&creationDesc, { dx12Obj->m_CpuAddress });

    return dx12Obj;
}

std::unique_ptr<Ether::Graphics::RhiUnorderedAccessView> Ether::Graphics::Dx12Device::CreateUnorderedAccessView(
    const RhiUnorderedAccessViewDesc& desc) const
{
    std::unique_ptr<Dx12UnorderedAccessView> dx12Obj = std::make_unique<Dx12UnorderedAccessView>();

    dx12Obj->m_CpuAddress = desc.m_TargetCpuAddress;
    const auto d3dResource = dynamic_cast<Dx12Resource*>(desc.m_Resource);

    auto creationDesc = Translate(desc);
    m_Device
        ->CreateUnorderedAccessView(d3dResource->m_Resource.Get(), nullptr, &creationDesc, { dx12Obj->m_CpuAddress });

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
        LogGraphicsFatal("Failed to serialize root signature");

    hr = m_Device->CreateRootSignature(
        1,
        rsBlob->GetBufferPointer(),
        rsBlob->GetBufferSize(),
        IID_PPV_ARGS(&dx12Obj->m_RootSignature));

    if (FAILED(hr))
        LogGraphicsFatal("Failed to create DirectX12 Root Signature");

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
        LogGraphicsFatal("Failed to create DirectX12 Pipeline State Object");

    return dx12Obj;
}

std::unique_ptr<Ether::Graphics::RhiAccelerationStructure> Ether::Graphics::Dx12Device::CreateAccelerationStructure(
    const RhiTopLevelAccelerationStructureDesc& desc) const
{
    std::unique_ptr<Dx12AccelerationStructure> dx12Obj = std::make_unique<Dx12AccelerationStructure>();
    VisualBatch* visualBatch = reinterpret_cast<VisualBatch*>(desc.m_VisualBatch);

    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs = {};
    inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
    inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE;
    inputs.NumDescs = 1;
    inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;

    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO info;
    m_Device->GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &info);
    dx12Obj->m_Size = info.ResultDataMaxSizeInBytes;

    RhiCommitedResourceDesc scratchBufferDesc = {};
    scratchBufferDesc.m_Name = "TLAS::ScratchBuffer";
    scratchBufferDesc.m_HeapType = RhiHeapType::Default;
    scratchBufferDesc.m_State = RhiResourceState::Common;
    scratchBufferDesc.m_ResourceDesc = RhiCreateBufferResourceDesc(info.ScratchDataSizeInBytes);
    scratchBufferDesc.m_ResourceDesc.m_Flag = RhiResourceFlag::AllowUnorderedAccess;
    scratchBufferDesc.m_ClearValue = nullptr;
    dx12Obj->m_ScratchBuffer = CreateCommittedResource(scratchBufferDesc);

    RhiCommitedResourceDesc dataBufferDesc = scratchBufferDesc;
    scratchBufferDesc.m_ResourceDesc = RhiCreateBufferResourceDesc(info.ResultDataMaxSizeInBytes);
    scratchBufferDesc.m_ResourceDesc.m_Flag = RhiResourceFlag::AllowUnorderedAccess;
    scratchBufferDesc.m_State = RhiResourceState::AccelerationStructure;
    dx12Obj->m_DataBuffer = CreateCommittedResource(scratchBufferDesc);

    RhiCommitedResourceDesc instanceBufferDesc = scratchBufferDesc;
    instanceBufferDesc.m_ResourceDesc = RhiCreateBufferResourceDesc(
        sizeof(D3D12_RAYTRACING_INSTANCE_DESC) * visualBatch->m_Visuals.size());
    instanceBufferDesc.m_State = RhiResourceState::GenericRead;
    instanceBufferDesc.m_HeapType = RhiHeapType::Upload;
    dx12Obj->m_InstanceDescBuffer = CreateCommittedResource(instanceBufferDesc);

    D3D12_RAYTRACING_INSTANCE_DESC* instanceDesc;
    dx12Obj->m_InstanceDescBuffer->Map(reinterpret_cast<void**>(&instanceDesc));
    for (uint64_t i = 0; i < visualBatch->m_Visuals.size(); ++i, ++instanceDesc)
    {
        instanceDesc->InstanceID = i;
        instanceDesc->InstanceContributionToHitGroupIndex = 0;
        instanceDesc->Flags = D3D12_RAYTRACING_INSTANCE_FLAG_NONE;
        instanceDesc->InstanceMask = 0xFF;
        instanceDesc->AccelerationStructure = visualBatch->m_Visuals[i]
                                                  .m_Mesh->GetAccelerationStructure()
                                                  .m_DataBuffer->GetGpuAddress();
    }
    dx12Obj->m_InstanceDescBuffer->Unmap();

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
    dx12Obj->m_GeometryDesc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
    dx12Obj->m_GeometryDesc.Triangles.VertexCount = mesh->GetNumVertices();
    dx12Obj->m_GeometryDesc.Flags = desc.m_IsOpaque ? D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE
                                                    : D3D12_RAYTRACING_GEOMETRY_FLAG_NONE;

    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs = {};
    inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
    inputs.NumDescs = 1;
    inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
    inputs.pGeometryDescs = &dx12Obj->m_GeometryDesc;
    if (desc.m_IsStatic)
        inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
    else
        inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_BUILD;
    dx12Obj->m_Inputs = inputs;

    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO info = {};
    m_Device->GetRaytracingAccelerationStructurePrebuildInfo(&inputs, &info);
    dx12Obj->m_Size = info.ResultDataMaxSizeInBytes;

    RhiCommitedResourceDesc scratchBufferDesc = {};
    scratchBufferDesc.m_Name = "BLAS::ScratchBuffer";
    scratchBufferDesc.m_HeapType = RhiHeapType::Default;
    scratchBufferDesc.m_State = RhiResourceState::Common;
    scratchBufferDesc.m_ResourceDesc = RhiCreateBufferResourceDesc(info.ScratchDataSizeInBytes);
    scratchBufferDesc.m_ResourceDesc.m_Flag = RhiResourceFlag::AllowUnorderedAccess;
    scratchBufferDesc.m_ClearValue = nullptr;
    dx12Obj->m_ScratchBuffer = CreateCommittedResource(scratchBufferDesc);

    RhiCommitedResourceDesc dataBufferDesc = scratchBufferDesc;
    dataBufferDesc.m_ResourceDesc = RhiCreateBufferResourceDesc(info.ResultDataMaxSizeInBytes);
    dataBufferDesc.m_ResourceDesc.m_Flag = RhiResourceFlag::AllowUnorderedAccess;
    dataBufferDesc.m_State = RhiResourceState::AccelerationStructure;
    dx12Obj->m_DataBuffer = CreateCommittedResource(dataBufferDesc);

    return dx12Obj;
}

std::unique_ptr<Ether::Graphics::RhiRaytracingPipelineState> Ether::Graphics::Dx12Device::CreateRaytracingPipelineState(
    const RhiRaytracingPipelineStateDesc& desc) const
{
    std::unique_ptr<Dx12RaytracingPipelineState> dx12Obj = std::make_unique<Dx12RaytracingPipelineState>();

    const wchar_t* raygenExportName[] = { desc.m_RayGenShaderName };
    const wchar_t* hitMissExportName[] = { desc.m_ClosestHitShaderName, desc.m_MissShaderName };
    const wchar_t* allExportName[] = { desc.m_ClosestHitShaderName, desc.m_MissShaderName, desc.m_RayGenShaderName };

    dx12Obj->PushLibrary(desc.m_LibraryShaderDesc);
    dx12Obj->PushHitProgram(desc.m_HitGroupName, nullptr, desc.m_ClosestHitShaderName);
    dx12Obj->PushLocalRootSignature(desc.m_RayGenRootSignature);
    dx12Obj->PushExportAssociation(raygenExportName, sizeof(raygenExportName) / sizeof(raygenExportName[0]));
    dx12Obj->PushLocalRootSignature(desc.m_HitMissRootSignature);
    dx12Obj->PushExportAssociation(hitMissExportName, sizeof(hitMissExportName) / sizeof(hitMissExportName[0]));
    dx12Obj->PushShaderConfig(desc.m_MaxAttributeSize, desc.m_MaxPayloadSize);
    dx12Obj->PushExportAssociation(allExportName, sizeof(allExportName) / sizeof(allExportName[0]));
    dx12Obj->PushPipelineConfig(desc.m_MaxRecursionDepth);
    dx12Obj->PushGlobalRootSignature(desc.m_GlobalRootSignature);

    D3D12_STATE_OBJECT_DESC stateObjDesc = {};
    stateObjDesc.NumSubobjects = dx12Obj->m_NumSubObjects;
    stateObjDesc.pSubobjects = dx12Obj->m_SubObjects;
    stateObjDesc.Type = D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE;

    HRESULT hr = m_Device->CreateStateObject(&stateObjDesc, IID_PPV_ARGS(&dx12Obj->m_PipelineState));
    if (FAILED(hr))
        LogGraphicsFatal("Failed to create DirectX12 Raytracing Pipeline State");

    return dx12Obj;
}

std::unique_ptr<Ether::Graphics::RhiRaytracingShaderBindingTable> Ether::Graphics::Dx12Device::
    CreateRaytracingShaderBindingTable(const RhiRaytracingShaderBindingTableDesc& desc) const
{
    uint32_t entrySize = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
    entrySize += desc.m_MaxRootSignatureSize;
    entrySize = AlignUp(entrySize, D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT);

    std::unique_ptr<Dx12RaytracingShaderBindingTable> dx12Obj = std::make_unique<Dx12RaytracingShaderBindingTable>(
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
    dynamic_cast<Dx12RaytracingPipelineState*>(desc.m_RaytracingPipelineState)
        ->m_PipelineState->QueryInterface(IID_PPV_ARGS(&stateObjectProperties));

    memcpy(
        mappedAddr,
        stateObjectProperties->GetShaderIdentifier(desc.m_RayGenShaderName),
        D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);

    constexpr uint8_t rootTableSize = 8;
    uint64_t* raygenDescriptorTable1 = (uint64_t*)(mappedAddr + D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES);
    uint64_t* raygenDescriptorTable2 = (uint64_t*)(mappedAddr + D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES + rootTableSize);

    *raygenDescriptorTable1 = desc.m_RayGenRootTableAddress1;
    *raygenDescriptorTable2 = desc.m_RayGenRootTableAddress2;

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

std::unique_ptr<Ether::Graphics::RhiResource> Ether::Graphics::Dx12Device::CreateCommittedResource(
    const RhiCommitedResourceDesc& desc) const
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
        LogGraphicsFatal("Failed to create DirectX12 commited resource (%s)", desc.m_Name.c_str());

    dx12Obj->m_Resource->SetName(ToWideString(desc.m_Name).c_str());
    dx12Obj->SetState(desc.m_State);
    return dx12Obj;
}

#endif // ETH_GRAPHICS_DX12
