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

#include "graphics/rhi/dx12/dx12accelerationstructure.h"
#include "graphics/rhi/dx12/dx12commandlist.h"
#include "graphics/rhi/dx12/dx12commandallocator.h"
#include "graphics/rhi/dx12/dx12descriptorheap.h"
#include "graphics/rhi/dx12/dx12pipelinestate.h"
#include "graphics/rhi/dx12/dx12resource.h"
#include "graphics/rhi/dx12/dx12resourceviews.h"
#include "graphics/rhi/dx12/dx12rootsignature.h"
#include "graphics/rhi/dx12/dx12translation.h"
#include "graphics/rhi/dx12/dx12raytracingpipelinestate.h"
#include "graphics/rhi/dx12/dx12raytracingshaderbindingtable.h"

#ifdef ETH_GRAPHICS_DX12

void Ether::Graphics::Dx12CommandList::Reset(const RhiCommandAllocator& commandAllocator)
{
    const auto allocator = dynamic_cast<const Dx12CommandAllocator&>(commandAllocator);
    HRESULT hr = m_CommandList->Reset(allocator.m_Allocator.Get(), nullptr);
    if (FAILED(hr))
        LogGraphicsFatal("Failed to reset DirectX12 Command List");
}

void Ether::Graphics::Dx12CommandList::Close()
{
    HRESULT hr = m_CommandList->Close();
    if (FAILED(hr))
        LogGraphicsFatal("Failed to close DirectX12 Command List");
}

void Ether::Graphics::Dx12CommandList::SetMarker(const std::string& name)
{
    PIXSetMarker(PIX_COLOR_INDEX(0), name.c_str());
}

void Ether::Graphics::Dx12CommandList::PushMarker(const std::string& name)
{
    PIXBeginEvent(m_CommandList.Get(), PIX_COLOR_INDEX(0), name.c_str());
}

void Ether::Graphics::Dx12CommandList::PopMarker()
{
    PIXEndEvent(m_CommandList.Get());
}

void Ether::Graphics::Dx12CommandList::SetDescriptorHeaps(const RhiSetDescriptorHeapsDesc& desc)
{
    // Only one heap of each type ([SRV/CBV/UAV] & [Sampler]) can be bound for each command list
    AssertGraphics(desc.m_NumHeaps <= 2, "A maximum of 2 heaps can be bound on DX12");
    ID3D12DescriptorHeap* heaps[2];

    for (int i = 0; i < desc.m_NumHeaps; ++i)
        heaps[i] = dynamic_cast<const Dx12DescriptorHeap*>(desc.m_Heaps[i])->m_Heap.Get();

    m_CommandList->SetDescriptorHeaps(desc.m_NumHeaps, heaps);
}

void Ether::Graphics::Dx12CommandList::SetPipelineState(const RhiPipelineState& pso)
{
    m_CommandList->SetPipelineState(dynamic_cast<const Dx12PipelineState&>(pso).m_PipelineState.Get());
}

void Ether::Graphics::Dx12CommandList::SetViewport(const RhiViewportDesc& viewport)
{
    D3D12_VIEWPORT dx12Viewport = Translate(viewport);
    m_CommandList->RSSetViewports(1, &dx12Viewport);
}

void Ether::Graphics::Dx12CommandList::SetScissorRect(const RhiScissorDesc& scissor)
{
    D3D12_RECT dx12Scissor = Translate(scissor);
    m_CommandList->RSSetScissorRects(1, &dx12Scissor);
}

void Ether::Graphics::Dx12CommandList::SetVertexBuffer(const RhiVertexBufferViewDesc& vertexBuffer)
{
    D3D12_VERTEX_BUFFER_VIEW dx12View = Translate(vertexBuffer);
    m_CommandList->IASetVertexBuffers(0, 1, &dx12View);
}

void Ether::Graphics::Dx12CommandList::SetIndexBuffer(const RhiIndexBufferViewDesc& indexBuffer)
{
    D3D12_INDEX_BUFFER_VIEW dx12View = Translate(indexBuffer);
    m_CommandList->IASetIndexBuffer(&dx12View);
}

void Ether::Graphics::Dx12CommandList::SetPrimitiveTopology(const RhiPrimitiveTopology& primitiveTopology)
{
    m_CommandList->IASetPrimitiveTopology(Translate(primitiveTopology));
}

void Ether::Graphics::Dx12CommandList::SetStencilRef(const RhiStencilValue& val)
{
    m_CommandList->OMSetStencilRef(val);
}

void Ether::Graphics::Dx12CommandList::SetRenderTargets(const RhiSetRenderTargetsDesc& desc)
{
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[8] = {};
    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = {};

    for (int i = 0; i < desc.m_NumRtv; ++i)
        rtvHandles[i] = { desc.m_RtvHandles[i]->GetCpuAddress() };

    if (desc.m_DsvHandle != nullptr)
        dsvHandle = { desc.m_DsvHandle->GetCpuAddress() };

    m_CommandList
        ->OMSetRenderTargets(desc.m_NumRtv, rtvHandles, false, desc.m_DsvHandle == nullptr ? nullptr : &dsvHandle);
}

void Ether::Graphics::Dx12CommandList::SetGraphicsRootConstantBuffer(uint32_t bindSlot, RhiGpuAddress resourceAddr)
{
    m_CommandList->SetGraphicsRootConstantBufferView(bindSlot, resourceAddr);
}

void Ether::Graphics::Dx12CommandList::SetGraphicRootSignature(const RhiRootSignature& rootSignature)
{
    const Dx12RootSignature& dx12Resource = dynamic_cast<const Dx12RootSignature&>(rootSignature);
    m_CommandList->SetGraphicsRootSignature(dx12Resource.m_RootSignature.Get());
}

void Ether::Graphics::Dx12CommandList::SetComputeRootDescriptorTable(
    uint32_t rootParameterIndex,
    RhiGpuAddress baseAddress)
{
    m_CommandList->SetComputeRootDescriptorTable(rootParameterIndex, { baseAddress });
}

void Ether::Graphics::Dx12CommandList::SetComputeRootSignature(const RhiRootSignature& rootSignature)
{
    const Dx12RootSignature& dx12Resource = dynamic_cast<const Dx12RootSignature&>(rootSignature);
    m_CommandList->SetComputeRootSignature(dx12Resource.m_RootSignature.Get());
}

void Ether::Graphics::Dx12CommandList::BuildAccelerationStructure(const RhiAccelerationStructure& as)
{
    const Dx12AccelerationStructure* dx12Obj = dynamic_cast<const Dx12AccelerationStructure*>(&as);

    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC asDesc = {};
    asDesc.Inputs = dx12Obj->m_Inputs;
    asDesc.DestAccelerationStructureData = dx12Obj->m_DataBuffer->GetGpuAddress();
    asDesc.ScratchAccelerationStructureData = dx12Obj->m_ScratchBuffer->GetGpuAddress();

    if (dx12Obj->m_InstanceDescBuffer != nullptr)
        asDesc.Inputs.InstanceDescs = dx12Obj->m_InstanceDescBuffer->GetGpuAddress();

    m_CommandList->BuildRaytracingAccelerationStructure(&asDesc, 0, nullptr);
    InsertUavBarrier(*as.m_DataBuffer);
}

void Ether::Graphics::Dx12CommandList::SetRaytracingPipelineState(const RhiRaytracingPipelineState& pso)
{
    const Dx12RaytracingPipelineState* dx12RtPso = dynamic_cast<const Dx12RaytracingPipelineState*>(&pso);
    m_CommandList->SetPipelineState1(dx12RtPso->m_PipelineState.Get());
}

void Ether::Graphics::Dx12CommandList::InsertUavBarrier(const RhiResource& uavResource)
{
    D3D12_RESOURCE_BARRIER uavBarrier = {};
    uavBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
    uavBarrier.UAV.pResource = (dynamic_cast<const Dx12Resource*>(&uavResource))->m_Resource.Get();
    m_CommandList->ResourceBarrier(1, &uavBarrier);
}

void Ether::Graphics::Dx12CommandList::TransitionResource(const RhiResourceTransitionDesc& desc)
{
    D3D12_RESOURCE_BARRIER barrier = Translate(desc);
    Dx12Resource* dx12Resource = dynamic_cast<Dx12Resource*>(desc.m_Resource);
    barrier.Transition.pResource = dx12Resource->m_Resource.Get();
    m_CommandList->ResourceBarrier(1, &barrier);
    desc.m_Resource->SetState(desc.m_ToState);
}

void Ether::Graphics::Dx12CommandList::CopyResource(const RhiResource& src, RhiResource& dest)
{
    const auto dx12SrcResource = (Dx12Resource*)&src;
    const auto dx12DstResource = (Dx12Resource*)&dest;

    m_CommandList->CopyResource(dx12DstResource->m_Resource.Get(), dx12SrcResource->m_Resource.Get());
}

void Ether::Graphics::Dx12CommandList::CopyBufferRegion(const RhiCopyBufferRegionDesc& desc)
{
    const auto dx12SrcResource = (Dx12Resource*)desc.m_Source;
    const auto dx12DstResource = (Dx12Resource*)desc.m_Destination;

    m_CommandList->CopyBufferRegion(
        dx12DstResource->m_Resource.Get(),
        desc.m_DestinationOffset,
        dx12SrcResource->m_Resource.Get(),
        desc.m_SourceOffset,
        desc.m_Size);
}

void Ether::Graphics::Dx12CommandList::CopyTextureRegion(const RhiCopyTextureRegionDesc& desc)
{
    const auto d3dIntermediateResource = (Dx12Resource*)desc.m_IntermediateResource;
    const auto d3dDestResource = (Dx12Resource*)desc.m_Destination;

    D3D12_SUBRESOURCE_DATA textureData = {};
    textureData.pData = desc.m_Data;
    textureData.RowPitch = desc.m_Width * desc.m_BytesPerPixel;
    textureData.SlicePitch = desc.m_Height * textureData.RowPitch;

    // d3dx12.h helper that will eventually call commandList->CopyTextureRegion()
    UpdateSubresources(
        m_CommandList.Get(),
        d3dDestResource->m_Resource.Get(),
        d3dIntermediateResource->m_Resource.Get(),
        desc.m_IntermediateResourceOffset, // offset
        0,                                 // subresource idx
        1,                                 // subresource count
        &textureData);
}

void Ether::Graphics::Dx12CommandList::ClearRenderTargetView(const RhiClearRenderTargetViewDesc& desc)
{
    m_CommandList->ClearRenderTargetView({ desc.m_RtvHandle->GetCpuAddress() }, desc.m_ClearColor.m_Data, 0, nullptr);
}

void Ether::Graphics::Dx12CommandList::ClearDepthStencilView(const RhiClearDepthStencilViewDesc& desc)
{
    m_CommandList->ClearDepthStencilView(
        { desc.m_DsvHandle->GetCpuAddress() },
        D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
        desc.m_ClearDepth,
        desc.m_ClearStencil,
        0,
        nullptr);
}

void Ether::Graphics::Dx12CommandList::DrawInstanced(const RhiDrawInstancedDesc& desc)
{
    m_CommandList->DrawInstanced(desc.m_VertexCount, desc.m_InstanceCount, desc.m_FirstVertex, desc.m_FirstInstance);
}

void Ether::Graphics::Dx12CommandList::DrawIndexedInstanced(const RhiDrawIndexedInstancedDesc& desc)
{
    m_CommandList->DrawIndexedInstanced(
        desc.m_IndexCount,
        desc.m_InstanceCount,
        desc.m_FirstIndex,
        desc.m_VertexOffset,
        desc.m_FirstInstance);
}

void Ether::Graphics::Dx12CommandList::DispatchRays(const RhiDispatchRaysDesc& desc)
{
    uint32_t entrySize = desc.m_ShaderBindingTable->GetTableEntrySize();

    D3D12_DISPATCH_RAYS_DESC dx12Desc = {};
    dx12Desc.Width = desc.m_DispatchWidth;
    dx12Desc.Height = desc.m_DispatchHeight;
    dx12Desc.Depth = desc.m_DispatchDepth;
    dx12Desc.RayGenerationShaderRecord.StartAddress = desc.m_ShaderBindingTable->GetGpuAddress();
    dx12Desc.RayGenerationShaderRecord.SizeInBytes = entrySize;

    dx12Desc.MissShaderTable.StartAddress = desc.m_ShaderBindingTable->GetGpuAddress() + entrySize;
    dx12Desc.MissShaderTable.SizeInBytes = entrySize;
    dx12Desc.MissShaderTable.StrideInBytes = entrySize;

    dx12Desc.HitGroupTable.StartAddress = desc.m_ShaderBindingTable->GetGpuAddress() + entrySize * 2;
    dx12Desc.HitGroupTable.SizeInBytes = entrySize;
    dx12Desc.HitGroupTable.StrideInBytes = entrySize;

    m_CommandList->DispatchRays(&dx12Desc);
}

#endif // ETH_GRAPHICS_DX12
