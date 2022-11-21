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

#include "graphics/rhi/dx12/dx12commandlist.h"
#include "graphics/rhi/dx12/dx12commandallocator.h"
#include "graphics/rhi/dx12/dx12descriptorheap.h"
//#include "graphics/rhi/dx12/dx12pipelinestate.h"
#include "graphics/rhi/dx12/dx12resource.h"
#include "graphics/rhi/dx12/dx12resourceviews.h"
//#include "graphics/rhi/dx12/dx12rootsignature.h"
#include "graphics/rhi/dx12/dx12translation.h"

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

//RhiResult Dx12CommandList::SetMarker(const char* name)
//{
//    PIXSetMarker(PIX_COLOR_INDEX(0), name);
//    return RhiResult::Success;
//}
//
//RhiResult Dx12CommandList::PushMarker(const char* name)
//{
//    PIXBeginEvent(m_CommandList.Get(), PIX_COLOR_INDEX(0), name);
//    return RhiResult::Success;
//}
//
//RhiResult Dx12CommandList::PopMarker()
//{
//    PIXEndEvent(m_CommandList.Get());
//    return RhiResult::Success;
//}
//
//RhiResult Dx12CommandList::SetViewport(const RhiViewportDesc& viewport)
//{
//    D3D12_VIEWPORT d3d12viewport = Translate(viewport);
//    m_CommandList->RSSetViewports(1, &d3d12viewport);
//    return RhiResult::Success;
//}
//
//RhiResult Dx12CommandList::SetScissor(const RhiScissorDesc& scissor)
//{
//    D3D12_RECT scissorRect = Translate(scissor);
//    m_CommandList->RSSetScissorRects(1, &scissorRect);
//    return RhiResult::Success;
//}
//
//RhiResult Dx12CommandList::SetStencilRef(const RhiStencilValue& val)
//{
//    m_CommandList->OMSetStencilRef(val);
//    return RhiResult::Success;
//}
//
//RhiResult Dx12CommandList::SetPrimitiveTopology(RhiPrimitiveTopology primitiveTopology)
//{
//    m_CommandList->IASetPrimitiveTopology(Translate(primitiveTopology));
//    return RhiResult::Success;
//}
//
//RhiResult Dx12CommandList::SetPipelineState(const RhiPipelineStateHandle pipelineState)
//{
//    const auto d3dPipelineState = pipelineState.As<Dx12PipelineState>();
//    m_CommandList->SetPipelineState(d3dPipelineState->m_PipelineState.Get());
//    return RhiResult::Success;
//}
//
//RhiResult Dx12CommandList::SetGraphicRootSignature(const RhiRootSignatureHandle rootSignature)
//{
//    const auto d3dRootSignature = rootSignature.As<Dx12RootSignature>();
//    m_CommandList->SetGraphicsRootSignature(d3dRootSignature->m_RootSignature.Get());
//    return RhiResult::Success;
//}
//
//RhiResult Dx12CommandList::SetVertexBuffer(const RhiVertexBufferViewDesc& vertexBuffer)
//{
//    const auto d3dResource = vertexBuffer.m_Resource.As<Dx12Resource>();
//    D3D12_VERTEX_BUFFER_VIEW d3dView = Translate(vertexBuffer);
//    d3dView.BufferLocation = d3dResource->GetGpuHandle().m_Ptr;
//    m_CommandList->IASetVertexBuffers(0, 1, &d3dView);
//    return RhiResult::Success;
//}
//
//RhiResult Dx12CommandList::SetIndexBuffer(const RhiIndexBufferViewDesc& indexBuffer)
//{
//    const auto d3dResource = indexBuffer.m_Resource.As<Dx12Resource>();
//    D3D12_INDEX_BUFFER_VIEW d3dView = Translate(indexBuffer);
//    d3dView.BufferLocation = d3dResource->GetGpuHandle().m_Ptr;
//    m_CommandList->IASetIndexBuffer(&d3dView);
//    return RhiResult::Success;
//}

void Ether::Graphics::Dx12CommandList::SetRenderTargets(const RhiSetRenderTargetsDesc& desc)
{
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[8];
    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = {};

    for (int i = 0; i < desc.m_NumRtv; ++i)
        rtvHandles[i] = Translate(desc.m_RtvHandles[i]->GetCpuAddress());

    if (desc.m_DsvHandle != nullptr)
        dsvHandle = Translate(desc.m_DsvHandle->GetCpuAddress());

    m_CommandList->OMSetRenderTargets
    (
        desc.m_NumRtv,
        rtvHandles,
        false,
        desc.m_DsvHandle == nullptr ? nullptr : &dsvHandle
    );
}

//RhiResult Dx12CommandList::SetRootConstant(const RhiSetRootConstantDesc& desc)
//{
//    m_CommandList->SetGraphicsRoot32BitConstant
//    (
//        desc.m_RootParameterIndex,
//        desc.m_SrcData,
//        desc.m_DestOffset
//    );
//
//    return RhiResult::Success;
//}
//
//RhiResult Dx12CommandList::SetRootConstants(const RhiSetRootConstantsDesc& desc)
//{
//    m_CommandList->SetGraphicsRoot32BitConstants
//    (
//        desc.m_RootParameterIndex,
//        desc.m_NumConstants,
//        desc.m_Data,
//        desc.m_DestOffset
//    );
//
//    return RhiResult::Success;
//}
//
//RhiResult Dx12CommandList::SetRootDescriptorTable(const RhiSetRootDescriptorTableDesc& desc)
//{
//    m_CommandList->SetGraphicsRootDescriptorTable
//    (
//        desc.m_RootParameterIndex,
//        Translate(desc.m_BaseSrvHandle)
//    );
//
//    return RhiResult::Success;
//}
//
//RhiResult Dx12CommandList::SetRootShaderResource(const RhiSetRootShaderResourceDesc& desc)
//{
//    m_CommandList->SetGraphicsRootShaderResourceView
//    (
//        desc.m_RootParameterIndex,
//        desc.m_Resource->GetGpuHandle().m_Ptr
//    );
//
//    return RhiResult::Success;
//}
//
//RhiResult Dx12CommandList::SetRootConstantBuffer(const RhiSetRootConstantBufferDesc& desc)
//{
//    m_CommandList->SetGraphicsRootConstantBufferView
//    (
//        desc.m_RootParameterIndex,
//        desc.m_Resource->GetGpuHandle().m_Ptr
//    );
//
//    return RhiResult::Success;
//}

void Ether::Graphics::Dx12CommandList::SetDescriptorHeaps(const RhiSetDescriptorHeapsDesc& desc)
{
    // Only one heap of each type ([SRV/CBV/UAV] & [Sampler]) can be bound for each command list
    AssertGraphics(desc.m_NumHeaps <= 2, "A maximum of 2 heaps can be bound on DX12");
    ID3D12DescriptorHeap* heaps[2];

    for (int i = 0; i < desc.m_NumHeaps; ++i)
        heaps[i] = dynamic_cast<const Dx12DescriptorHeap*>(desc.m_Heaps[i])->m_Heap.Get();

    m_CommandList->SetDescriptorHeaps(desc.m_NumHeaps, heaps);
}

void Ether::Graphics::Dx12CommandList::ClearRenderTargetView(const RhiClearRenderTargetViewDesc& desc)
{
    m_CommandList->ClearRenderTargetView
    (
        Translate(desc.m_RtvHandle->GetCpuAddress()),
        desc.m_ClearColor.m_Data,
        0,
        nullptr
    );
}


//RhiResult Dx12CommandList::ClearDepthStencilView(const RhiClearDepthStencilViewDesc& desc)
//{
//    m_CommandList->ClearDepthStencilView
//    (
//        Translate(desc.m_DsvHandle->GetCpuAddress()),
//        D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
//        desc.m_ClearDepth,
//        desc.m_ClearStencil,
//        0,
//        nullptr
//    );
//
//    return RhiResult::Success;
//}
//
//RhiResult Dx12CommandList::CopyBufferRegion(const RhiCopyBufferRegionDesc& desc)
//{
//    const auto d3dSrcResource = desc.m_Source.As<Dx12Resource>();
//    const auto d3dDestResource = desc.m_Destination.As<Dx12Resource>();
//
//    m_CommandList->CopyBufferRegion
//    (
//        d3dDestResource->m_Resource.Get(),
//        desc.m_DestinationOffset,
//        d3dSrcResource->m_Resource.Get(),
//        desc.m_SourceOffset,
//        desc.m_Size
//    );
//
//    return RhiResult::Success;
//}
//
//RhiResult Dx12CommandList::CopyTextureRegion(const RhiCopyTextureRegionDesc& desc)
//{
//    const auto d3dIntermediateResource = desc.m_IntermediateResource.As<Dx12Resource>();
//    const auto d3dDestResource = desc.m_Destination.As<Dx12Resource>();
//
//    D3D12_SUBRESOURCE_DATA textureData = {};
//    textureData.pData = desc.m_Data;
//    textureData.RowPitch = desc.m_Width * desc.m_BytesPerPixel;
//    textureData.SlicePitch = desc.m_Height * textureData.RowPitch;
//
//    // d3dx12.h helper that will eventually call commandList->CopyTextureRegion()
//    UpdateSubresources
//    (
//        m_CommandList.Get(),
//        d3dDestResource->m_Resource.Get(),
//        d3dIntermediateResource->m_Resource.Get(),
//        desc.m_IntermediateResourceOffset, // offset
//        0, // subresource idx
//        1, // subresource count
//        &textureData
//    );
//
//    return RhiResult::Success;
//}
//
void Ether::Graphics::Dx12CommandList::TransitionResource(const RhiResourceTransitionDesc& desc)
{
    D3D12_RESOURCE_BARRIER barrier = Translate(desc);
    Dx12Resource* dx12Resource = dynamic_cast<Dx12Resource*>(desc.m_Resource);
    barrier.Transition.pResource = dx12Resource->m_Resource.Get();
    m_CommandList->ResourceBarrier(1, &barrier);
    desc.m_Resource->SetState(desc.m_ToState);
}

//RhiResult Dx12CommandList::TransitionResource(const RhiResourceTransitionDesc& desc)
//{
//}
//
//RhiResult Dx12CommandList::DrawInstanced(const RhiDrawInstancedDesc& desc)
//{
//    m_CommandList->DrawInstanced
//    (
//        desc.m_VertexCount,
//        desc.m_InstanceCount,
//        desc.m_FirstVertex,
//        desc.m_FirstInstance
//    );
//
//    return RhiResult::Success;
//}
//
//RhiResult Dx12CommandList::DrawIndexedInstanced(const RhiDrawIndexedInstancedDesc& desc)
//{
//    m_CommandList->DrawIndexedInstanced
//    (
//        desc.m_IndexCount,
//        desc.m_InstanceCount,
//        desc.m_FirstIndex,
//        desc.m_VertexOffset,
//        desc.m_FirstInstance
//    );
//
//    return RhiResult::Success;
//}
//
void Ether::Graphics::Dx12CommandList::Reset(const RhiCommandAllocator& commandAllocator)
{
    const auto allocator = dynamic_cast<const Dx12CommandAllocator&>(commandAllocator);
    HRESULT hr = m_CommandList->Reset(allocator.m_Allocator.Get(), nullptr);
    if (FAILED(hr))
        LogGraphicsError("Failed to reset Dx12 Command List");
}

void Ether::Graphics::Dx12CommandList::Close()
{
    HRESULT hr = m_CommandList->Close();
	if (FAILED(hr))
		LogGraphicsError("Failed to close Dx12 Command List");
}

