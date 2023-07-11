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
#include "graphics/rhi/dx12/dx12rootsignature.h"
#include "graphics/rhi/dx12/dx12translation.h"
#include "graphics/rhi/dx12/dx12raytracingpipelinestate.h"
#include "graphics/rhi/dx12/dx12raytracingshaderbindingtable.h"

#include "graphics/graphiccore.h"

#ifdef ETH_GRAPHICS_DX12

Ether::Graphics::Dx12CommandList::Dx12CommandList(RhiCommandType type)
    : RhiCommandList(type)
{
}

Ether::Graphics::Dx12CommandList::~Dx12CommandList()
{
    if (m_CommandAllocator != nullptr)
        m_CommandAllocatorPool->DiscardAllocator(*m_CommandAllocator);
}

void Ether::Graphics::Dx12CommandList::Reset()
{
    if (m_CommandAllocator != nullptr)
        m_CommandAllocatorPool->DiscardAllocator(*m_CommandAllocator);

    m_CommandAllocator = &m_CommandAllocatorPool->RequestAllocator();

    const auto allocator = dynamic_cast<const Dx12CommandAllocator&>(*m_CommandAllocator);
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

void Ether::Graphics::Dx12CommandList::SetDescriptorHeaps(const RhiDescriptorHeap& srvHeap, const RhiDescriptorHeap* samplerHeap)
{
    // Only one heap of each type ([SRV/CBV/UAV] & [Sampler]) can be bound for each command list
    ID3D12DescriptorHeap* heaps[2];
    heaps[0] = dynamic_cast<const Dx12DescriptorHeap*>(&srvHeap)->m_Heap.Get();

    if (samplerHeap != nullptr)
        heaps[1] = dynamic_cast<const Dx12DescriptorHeap*>(samplerHeap)->m_Heap.Get();

    m_CommandList->SetDescriptorHeaps(samplerHeap == nullptr ? 1 : 2, heaps);
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

void Ether::Graphics::Dx12CommandList::SetRenderTargets(
    const RhiRenderTargetView* rtvs,
    uint32_t numRtvs,
    const RhiDepthStencilView* dsv)
{
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[8] = {};
    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = {};

    for (int i = 0; i < numRtvs; ++i)
        rtvHandles[i] = { rtvs[i].GetCpuAddress() };

    if (dsv != nullptr)
        dsvHandle = { dsv->GetCpuAddress() };

    m_CommandList->OMSetRenderTargets(numRtvs, rtvHandles, false, dsv == nullptr ? nullptr : &dsvHandle);
}

void Ether::Graphics::Dx12CommandList::SetGraphicRootSignature(const RhiRootSignature& rootSignature)
{
    const Dx12RootSignature& dx12Resource = dynamic_cast<const Dx12RootSignature&>(rootSignature);
    m_CommandList->SetGraphicsRootSignature(dx12Resource.m_RootSignature.Get());
}

void Ether::Graphics::Dx12CommandList::SetGraphicsRootConstant(
    uint32_t rootParameterIndex,
    uint32_t data,
    uint32_t destOffset)
{
    m_CommandList->SetGraphicsRoot32BitConstant(rootParameterIndex, data, destOffset);
}

void Ether::Graphics::Dx12CommandList::SetGraphicsRootConstantBufferView(
    uint32_t rootParameterIndex,
    RhiGpuAddress resourceAddr)
{
    m_CommandList->SetGraphicsRootConstantBufferView(rootParameterIndex, resourceAddr);
}

void Ether::Graphics::Dx12CommandList::SetGraphicsRootShaderResourceView(
    uint32_t rootParameterIndex,
    RhiGpuAddress resourceAddr)
{
    m_CommandList->SetGraphicsRootShaderResourceView(rootParameterIndex, resourceAddr);
}

void Ether::Graphics::Dx12CommandList::SetGraphicsRootUnorderedAccessView(
    uint32_t rootParameterIndex,
    RhiGpuAddress resourceAddr)
{
    m_CommandList->SetGraphicsRootUnorderedAccessView(rootParameterIndex, resourceAddr);
}

void Ether::Graphics::Dx12CommandList::SetGraphicsRootDescriptorTable(
    uint32_t rootParameterIndex,
    RhiGpuAddress baseAddress)
{
    m_CommandList->SetGraphicsRootDescriptorTable(rootParameterIndex, { baseAddress });
}

void Ether::Graphics::Dx12CommandList::SetComputeRootSignature(const RhiRootSignature& rootSignature)
{
    const Dx12RootSignature& dx12Resource = dynamic_cast<const Dx12RootSignature&>(rootSignature);
    m_CommandList->SetComputeRootSignature(dx12Resource.m_RootSignature.Get());
}

void Ether::Graphics::Dx12CommandList::SetComputeRootConstant(
    uint32_t rootParameterIndex,
    uint32_t data,
    uint32_t destOffset)
{
    m_CommandList->SetComputeRoot32BitConstant(rootParameterIndex, data, destOffset);
}

void Ether::Graphics::Dx12CommandList::SetComputeRootConstantBufferView(
    uint32_t rootParameterIndex,
    RhiGpuAddress resourceAddr)
{
    m_CommandList->SetComputeRootConstantBufferView(rootParameterIndex, resourceAddr);
}

void Ether::Graphics::Dx12CommandList::SetComputeRootShaderResourceView(
    uint32_t rootParameterIndex,
    RhiGpuAddress resourceAddr)
{
    m_CommandList->SetComputeRootShaderResourceView(rootParameterIndex, resourceAddr);
}

void Ether::Graphics::Dx12CommandList::SetComputeRootUnorderedAccessView(
    uint32_t rootParameterIndex,
    RhiGpuAddress resourceAddr)
{
    m_CommandList->SetComputeRootUnorderedAccessView(rootParameterIndex, resourceAddr);
}

void Ether::Graphics::Dx12CommandList::SetComputeRootDescriptorTable(
    uint32_t rootParameterIndex,
    RhiGpuAddress baseAddress)
{
    m_CommandList->SetComputeRootDescriptorTable(rootParameterIndex, { baseAddress });
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

void Ether::Graphics::Dx12CommandList::TransitionResource(RhiResource& resource, RhiResourceState newState)
{
    D3D12_RESOURCE_BARRIER dx12Desc = {};
    dx12Desc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    dx12Desc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    dx12Desc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    dx12Desc.Transition.StateBefore = Translate(resource.GetCurrentState());
    dx12Desc.Transition.StateAfter = Translate(newState);

    Dx12Resource* dx12Resource = dynamic_cast<Dx12Resource*>(&resource);
    dx12Desc.Transition.pResource = dx12Resource->m_Resource.Get();
    m_CommandList->ResourceBarrier(1, &dx12Desc);
    resource.SetState(newState);
}

void Ether::Graphics::Dx12CommandList::CopyResource(const RhiResource& src, RhiResource& dest)
{
    const auto dx12SrcResource = (Dx12Resource*)&src;
    const auto dx12DstResource = (Dx12Resource*)&dest;

    m_CommandList->CopyResource(dx12DstResource->m_Resource.Get(), dx12SrcResource->m_Resource.Get());
}

void Ether::Graphics::Dx12CommandList::CopyBufferRegion(
    const RhiResource& src,
    RhiResource& dest,
    uint32_t size,
    uint32_t srcOff,
    uint32_t destOff)
{
    const auto dx12SrcResource = (Dx12Resource*)&src;
    const auto dx12DstResource = (Dx12Resource*)&dest;

    m_CommandList->CopyBufferRegion(dx12DstResource->m_Resource.Get(), destOff, dx12SrcResource->m_Resource.Get(), srcOff, size);
}

void Ether::Graphics::Dx12CommandList::CopyTexture(
    RhiResource& scratch,
    RhiResource& dest,
    void** data,
    uint32_t numMips,
    uint32_t width,
    uint32_t height,
    uint32_t bytesPerPixel)
{
    const auto dx12ScratchResource = (Dx12Resource*)&scratch;
    const auto dx12DstResource = (Dx12Resource*)&dest;

    std::vector<D3D12_SUBRESOURCE_DATA> allMipsData;

    for (uint32_t i = 0; i < numMips; ++i)
    {
        D3D12_SUBRESOURCE_DATA mipData = {};
        mipData.pData = data[i];
        mipData.RowPitch = width * std::pow(0.5, i) * bytesPerPixel;
        mipData.SlicePitch = height * std::pow(0.5, i) * mipData.RowPitch;
        allMipsData.push_back(mipData);
    }

    // d3dx12.h helper that will eventually call commandList->CopyTextureRegion()
    UpdateSubresources(
        m_CommandList.Get(),
        dx12DstResource->m_Resource.Get(),
        dx12ScratchResource->m_Resource.Get(),
        0,
        0,
        numMips,
        allMipsData.data());
}

void Ether::Graphics::Dx12CommandList::ClearRenderTargetView(
    const RhiRenderTargetView rtv,
    const ethVector4& clearColor)
{
    m_CommandList->ClearRenderTargetView({ rtv.GetCpuAddress() }, clearColor.m_Data, 0, nullptr);
}

void Ether::Graphics::Dx12CommandList::ClearDepthStencilView(const RhiDepthStencilView dsv, float depth, float stencil)
{
    m_CommandList->ClearDepthStencilView(
        { dsv.GetCpuAddress() },
        D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
        depth,
        stencil,
        0,
        nullptr);
}

void Ether::Graphics::Dx12CommandList::DrawInstanced(
    uint32_t numVert,
    uint32_t numInst,
    uint32_t firstVert,
    uint32_t firstInst)
{
    m_CommandList->DrawInstanced(numVert, numInst, firstVert, firstInst);
}

void Ether::Graphics::Dx12CommandList::DrawIndexedInstanced(
    uint32_t numIndices,
    uint32_t numInst,
    uint32_t firstIdx,
    uint32_t stride,
    uint32_t firstInst)
{
    m_CommandList->DrawIndexedInstanced(numIndices, numInst, firstIdx, stride, firstInst);
}

void Ether::Graphics::Dx12CommandList::DispatchRays(uint32_t x, uint32_t y, uint32_t z, const RhiResource* bindTable)
{
    const Dx12RaytracingShaderBindingTable* sbt = dynamic_cast<const Dx12RaytracingShaderBindingTable*>(bindTable);

    D3D12_DISPATCH_RAYS_DESC dx12Desc = {};
    dx12Desc.Width = x;
    dx12Desc.Height = y;
    dx12Desc.Depth = z;
    dx12Desc.RayGenerationShaderRecord.StartAddress = bindTable->GetGpuAddress();
    dx12Desc.RayGenerationShaderRecord.SizeInBytes = sbt->GetTableEntrySize();

    dx12Desc.MissShaderTable.StartAddress = bindTable->GetGpuAddress() + sbt->GetTableEntrySize();
    dx12Desc.MissShaderTable.SizeInBytes = sbt->GetTableEntrySize();
    dx12Desc.MissShaderTable.StrideInBytes = sbt->GetTableEntrySize();

    dx12Desc.HitGroupTable.StartAddress = bindTable->GetGpuAddress() + sbt->GetTableEntrySize() * 2;
    dx12Desc.HitGroupTable.SizeInBytes = sbt->GetTableEntrySize();
    dx12Desc.HitGroupTable.StrideInBytes = sbt->GetTableEntrySize();

    m_CommandList->DispatchRays(&dx12Desc);
}

#endif // ETH_GRAPHICS_DX12
