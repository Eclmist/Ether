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

#include "dx12commandlist.h"
#include "dx12commandallocator.h"
#include "dx12descriptorheap.h"
#include "dx12pipelinestate.h"
#include "dx12resource.h"
#include "dx12resourceviews.h"
#include "dx12rootsignature.h"
#include "dx12translation.h"

ETH_NAMESPACE_BEGIN

RhiResult Dx12CommandList::SetMarker(const char* name)
{
    // Requires pix event runtime
    return RhiResult::NotSupported;
}


RhiResult Dx12CommandList::PushMarker(const char* name)
{
    // Requires pix event runtime
    return RhiResult::NotSupported;
}

RhiResult Dx12CommandList::PopMarker()
{
    // Requires pix event runtime
    return RhiResult::NotSupported;
}

RhiResult Dx12CommandList::SetViewport(const RhiViewportDesc& viewport)
{
    D3D12_VIEWPORT d3d12viewport = Translate(viewport);
    m_CommandList->RSSetViewports(1, &d3d12viewport);
    return RhiResult::Success;
}

RhiResult Dx12CommandList::SetScissor(const RhiScissorDesc& scissor)
{
    D3D12_RECT scissorRect = Translate(scissor);
    m_CommandList->RSSetScissorRects(1, &scissorRect);
    return RhiResult::Success;
}

RhiResult Dx12CommandList::SetStencilRef(const RhiStencilValue& val)
{
    m_CommandList->OMSetStencilRef(val);
    return RhiResult::Success;
}

RhiResult Dx12CommandList::SetPrimitiveTopology(RhiPrimitiveTopology primitiveTopology)
{
    m_CommandList->IASetPrimitiveTopology(Translate(primitiveTopology));
    return RhiResult::Success;
}

RhiResult Dx12CommandList::SetPipelineState(const RhiPipelineStateHandle pipelineState)
{
    const auto d3dPipelineState = pipelineState.As<Dx12PipelineState>();
    m_CommandList->SetPipelineState(d3dPipelineState->m_PipelineState.Get());
    return RhiResult::Success;
}

RhiResult Dx12CommandList::SetGraphicRootSignature(const RhiRootSignatureHandle rootSignature)
{
    const auto d3dRootSignature = rootSignature.As<Dx12RootSignature>();
    m_CommandList->SetGraphicsRootSignature(d3dRootSignature->m_RootSignature.Get());
    return RhiResult::Success;
}

RhiResult Dx12CommandList::SetVertexBuffer(const RhiVertexBufferViewDesc& vertexBuffer)
{
    const auto d3dResource = vertexBuffer.m_Resource.As<Dx12Resource>();
    D3D12_VERTEX_BUFFER_VIEW d3dView = Translate(vertexBuffer);
    d3dView.BufferLocation = d3dResource->GetGpuHandle().m_Ptr;
    m_CommandList->IASetVertexBuffers(0, 1, &d3dView);
    return RhiResult::Success;
}

RhiResult Dx12CommandList::SetIndexBuffer(const RhiIndexBufferViewDesc& indexBuffer)
{
    const auto d3dResource = indexBuffer.m_Resource.As<Dx12Resource>();
    D3D12_INDEX_BUFFER_VIEW d3dView = Translate(indexBuffer);
    d3dView.BufferLocation = d3dResource->GetGpuHandle().m_Ptr;
    m_CommandList->IASetIndexBuffer(&d3dView);
    return RhiResult::Success;
}

RhiResult Dx12CommandList::SetRenderTargets(const RhiSetRenderTargetsDesc& desc)
{
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[8];

    for (int i = 0; i < desc.m_NumRTV; ++i)
		rtvHandles[i] = Translate(desc.m_RTVHandles[i]->GetCpuAddress());

    m_CommandList->OMSetRenderTargets
    (
        desc.m_NumRTV,
        rtvHandles,
        false,
        desc.m_DSVHandle.IsNull() ? nullptr : &Translate(desc.m_DSVHandle->GetCpuAddress())
    );

    return RhiResult::Success;
}

RhiResult Dx12CommandList::SetRootConstants(const RhiSetRootConstantsDesc& desc)
{
    m_CommandList->SetGraphicsRoot32BitConstants
    (
        desc.m_RootParameterIndex,
        desc.m_NumConstants,
        desc.m_Data,
        desc.m_DestOffset
    );

    return RhiResult::Success;
}

RhiResult Dx12CommandList::SetRootDescriptorTable(const RhiSetRootDescriptorTableDesc& desc)
{
    m_CommandList->SetGraphicsRootDescriptorTable
    (
        desc.m_RootParameterIndex,
        Translate(desc.m_BaseSRVHandle->GetGpuHandle())
    );

    return RhiResult::Success;
}

RhiResult Dx12CommandList::SetRootShaderResource(const RhiSetRootShaderResourceDesc& desc)
{
    m_CommandList->SetGraphicsRootShaderResourceView
    (
        desc.m_RootParameterIndex,
        desc.m_Resource->GetGpuHandle().m_Ptr
    );

    return RhiResult::Success;
}

RhiResult Dx12CommandList::SetRootConstantBuffer(const RhiSetRootConstantBufferDesc& desc)
{
    m_CommandList->SetGraphicsRootConstantBufferView
    (
        desc.m_RootParameterIndex,
        desc.m_Resource->GetGpuHandle().m_Ptr
    );

    return RhiResult::Success;
}

RhiResult Dx12CommandList::SetDescriptorHeaps(const RhiSetDescriptorHeapsDesc& desc)
{
    std::vector<ID3D12DescriptorHeap*> heaps;
    for (int i = 0; i < desc.m_NumHeaps; ++i)
        heaps.push_back(desc.m_Heaps[i].As<Dx12DescriptorHeap>()->m_Heap.Get());

    m_CommandList->SetDescriptorHeaps(desc.m_NumHeaps, heaps.data());

    return RhiResult::Success;
}

RhiResult Dx12CommandList::ClearRenderTargetView(const RhiClearRenderTargetViewDesc& desc)
{
    float clearColor[] = 
    {
        desc.m_ClearColor.x,
        desc.m_ClearColor.y,
        desc.m_ClearColor.z,
        desc.m_ClearColor.w
    };

    m_CommandList->ClearRenderTargetView
    (
        Translate(desc.m_RTVHandle->GetCpuAddress()),
        clearColor,
        0,
        nullptr
    );

    return RhiResult::Success;
}

RhiResult Dx12CommandList::ClearDepthStencilView(const RhiClearDepthStencilViewDesc& desc)
{
    m_CommandList->ClearDepthStencilView
    (
        Translate(desc.m_DSVHandle->GetCpuAddress()),
        D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
        desc.m_ClearDepth,
        desc.m_ClearStencil,
        0,
        nullptr
    );

    return RhiResult::Success;
}

RhiResult Dx12CommandList::CopyBufferRegion(const RhiCopyBufferRegionDesc& desc)
{
    const auto d3dSrcResource = desc.m_Source.As<Dx12Resource>();
    const auto d3dDestResource = desc.m_Destination.As<Dx12Resource>();

    m_CommandList->CopyBufferRegion
    (
        d3dDestResource->m_Resource.Get(),
        desc.m_DestinationOffset,
        d3dSrcResource->m_Resource.Get(),
        desc.m_SourceOffset,
        desc.m_Size
    );

    return RhiResult::Success;
}

RhiResult Dx12CommandList::CopyTextureRegion(const RhiCopyTextureRegionDesc& desc)
{
    const auto d3dSrcResource = desc.m_Source.As<Dx12Resource>();
    const auto d3dDestResource = desc.m_Destination.As<Dx12Resource>();

    D3D12_SUBRESOURCE_DATA textureData = {};
    textureData.pData = desc.m_Data;
    textureData.RowPitch = desc.m_Width * desc.m_BytesPerPixel;
    textureData.SlicePitch = desc.m_Height * textureData.RowPitch;

    // d3dx12.h helper that will eventually call commandList->CopyTextureRegion()
    UpdateSubresources
    (
        m_CommandList.Get(),
        d3dDestResource->m_Resource.Get(),
        d3dSrcResource->m_Resource.Get(),
        0, // offset
        0, // subresource idx
        1, // subresource count
        &textureData
    );

    return RhiResult::Success;
}

RhiResult Dx12CommandList::TransitionResource(const RhiResourceTransitionDesc& desc)
{
    D3D12_RESOURCE_BARRIER barrier = Translate(desc);
    const auto d3dResource = desc.m_Resource.As<Dx12Resource>();
    barrier.Transition.pResource = d3dResource->m_Resource.Get();
    m_CommandList->ResourceBarrier(1, &barrier);
    return RhiResult::Success;
}

RhiResult Dx12CommandList::DrawInstanced(const RhiDrawInstancedDesc& desc)
{
    m_CommandList->DrawInstanced
    (
        desc.m_VertexCount,
        desc.m_InstanceCount,
        desc.m_FirstVertex,
        desc.m_FirstInstance
    );

    return RhiResult::Success;
}

RhiResult Dx12CommandList::DrawIndexedInstanced(const RhiDrawIndexedInstancedDesc& desc)
{
    m_CommandList->DrawIndexedInstanced
    (
        desc.m_IndexCount,
        desc.m_InstanceCount,
        desc.m_FirstIndex,
        desc.m_VertexOffset,
        desc.m_FirstInstance
    );

    return RhiResult::Success;
}

RhiResult Dx12CommandList::Reset(const RhiCommandAllocatorHandle commandAllocator)
{
    const auto allocator = commandAllocator.As<Dx12CommandAllocator>();
    HRESULT hr = m_CommandList->Reset(allocator->m_Allocator.Get(), nullptr);
    return TO_RHI_RESULT(hr);
}

RhiResult Dx12CommandList::Close()
{
    HRESULT hr = m_CommandList->Close();
    return TO_RHI_RESULT(hr);
}

ETH_NAMESPACE_END
