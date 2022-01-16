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

#include "d3d12commandlist.h"
#include "d3d12commandallocator.h"
#include "d3d12pipelinestate.h"
#include "d3d12resource.h"
#include "d3d12resourceviews.h"
#include "d3d12rootsignature.h"
#include "d3d12translation.h"

ETH_NAMESPACE_BEGIN

RHIResult D3D12CommandList::SetViewport(const RHIViewportDesc& viewport)
{
    D3D12_VIEWPORT d3d12viewport = Translate(viewport);
    m_CommandList->RSSetViewports(1, &d3d12viewport);
    return RHIResult::Success;
}

RHIResult D3D12CommandList::SetScissor(const RHIScissorDesc& scissor)
{
    D3D12_RECT scissorRect = Translate(scissor);
    m_CommandList->RSSetScissorRects(1, &scissorRect);
    return RHIResult::Success;
}

RHIResult D3D12CommandList::SetPrimitiveTopology(RHIPrimitiveTopology primitiveTopology)
{
    m_CommandList->IASetPrimitiveTopology(Translate(primitiveTopology));
    return RHIResult::Success;
}

RHIResult D3D12CommandList::SetPipelineState(const RHIPipelineStateHandle pipelineState)
{
    const auto d3dPipelineState = pipelineState.As<D3D12PipelineState>();
    m_CommandList->SetPipelineState(d3dPipelineState->m_PipelineState.Get());
    return RHIResult::Success;
}

RHIResult D3D12CommandList::SetGraphicRootSignature(const RHIRootSignatureHandle rootSignature)
{
    const auto d3dRootSignature = rootSignature.As<D3D12RootSignature>();
    m_CommandList->SetGraphicsRootSignature(d3dRootSignature->m_RootSignature.Get());
    return RHIResult::Success;
}

RHIResult D3D12CommandList::SetVertexBuffer(const RHIBufferHandle vertexBuffer)
{
    return RHIResult::NotSupported;
}

RHIResult D3D12CommandList::SetIndexBuffer(const RHIBufferHandle indexBuffer)
{
    return RHIResult::NotSupported;
}

RHIResult D3D12CommandList::SetRenderTargets(const RHISetRenderTargetsDesc& desc)
{
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[8];
    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandles[8];

    for (int i = 0; i < desc.m_NumRTV; ++i)
    {
        if (!desc.m_RTVHandles[0].IsNull())
			rtvHandles[i] = Translate(desc.m_RTVHandles[0]->GetCPUHandle());

        if (!desc.m_DSVHandles[0].IsNull())
			dsvHandles[i] = Translate(desc.m_DSVHandles[0]->GetCPUHandle());
    }
    
    m_CommandList->OMSetRenderTargets
    (
        desc.m_NumRTV,
        rtvHandles,
        false,
        dsvHandles
    );

    return RHIResult::Success;
}

RHIResult D3D12CommandList::SetRootConstants(const RHISetRootConstantsDesc& desc)
{
    m_CommandList->SetComputeRoot32BitConstants
    (
        desc.m_RootParameterIndex,
        desc.m_NumConstants,
        desc.m_Data,
        desc.m_DestOffset
    );

    return RHIResult::Success;
}

RHIResult D3D12CommandList::ClearRenderTargetView(const RHIClearRenderTargetViewDesc& desc)
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
        Translate(desc.m_RTVHandle->GetCPUHandle()),
        clearColor,
        0,
        nullptr
    );

    return RHIResult::Success;
}

RHIResult D3D12CommandList::ClearDepthStencilView(const RHIClearDepthStencilViewDesc& desc)
{
    m_CommandList->ClearDepthStencilView
    (
        Translate(desc.m_DSVHandle->GetCPUHandle()),
        D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
        desc.m_ClearDepth,
        desc.m_ClearStencil,
        0,
        nullptr
    );

    return RHIResult::Success;
}

RHIResult D3D12CommandList::CopyBufferRegion(const RHICopyBufferRegionDesc& desc)
{
    const auto d3dSrcResource = desc.m_Source.As<D3D12Resource>();
    const auto d3dDestResource = desc.m_Destination.As<D3D12Resource>();

    m_CommandList->CopyBufferRegion
    (
        d3dDestResource->m_Resource.Get(),
        desc.m_DestinationOffset,
        d3dSrcResource->m_Resource.Get(),
        desc.m_SourceOffset,
        desc.m_Size
    );

    return RHIResult::Success;
}

RHIResult D3D12CommandList::TransitionResource(const RHIResourceTransitionDesc& desc)
{
    D3D12_RESOURCE_BARRIER barrier = Translate(desc);
    const auto d3dResource = desc.m_Resource.As<D3D12Resource>();
    barrier.Transition.pResource = d3dResource->m_Resource.Get();
    m_CommandList->ResourceBarrier(1, &barrier);
    return RHIResult::Success;
}

RHIResult D3D12CommandList::DrawIndexedInstanced(const RHIDrawIndexedInstancedDesc& desc)
{
    m_CommandList->DrawIndexedInstanced
    (
        desc.m_IndexCount,
        desc.m_InstanceCount,
        desc.m_FirstIndex,
        desc.m_VertexOffset,
        desc.m_FirstInstance
    );

    return RHIResult::Success;
}

RHIResult D3D12CommandList::Reset(const RHICommandAllocatorHandle commandAllocator)
{
    const auto allocator = commandAllocator.As<D3D12CommandAllocator>();
    HRESULT hr = m_CommandList->Reset(allocator->m_Allocator.Get(), nullptr);
    return TO_RHI_RESULT(hr);
}

RHIResult D3D12CommandList::Close()
{
    HRESULT hr = m_CommandList->Close();
    return TO_RHI_RESULT(hr);
}

ETH_NAMESPACE_END
