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

#include "graphiccontext.h"
#include "graphic/resource/bufferresource.h"

ETH_NAMESPACE_BEGIN

GraphicContext::GraphicContext(D3D12_COMMAND_LIST_TYPE type)
    : m_Type(type)
    , m_ViewMatrix(DirectX::XMMatrixIdentity())
    , m_ProjectionMatrix(DirectX::XMMatrixIdentity())
{
    GraphicCore::GetCommandManager().CreateCommandList(type, &m_CommandList, &m_CommandAllocator);
}

GraphicContext::~GraphicContext()
{
}

void GraphicContext::Reset()
{
    m_CommandAllocator = &GraphicCore::GetCommandManager().RequestAllocator(m_Type);
    m_CommandList->Reset(m_CommandAllocator, nullptr);
}

CommandQueue& GraphicContext::GetCommandQueue() const
{
    return GraphicCore::GetCommandManager().GetQueue(m_Type);
}

uint64_t GraphicContext::GetCompletionFenceValue() const
{
    return GetCommandQueue().GetCompletionFenceValue();
}

void GraphicContext::ClearColor(TextureResource& texture, ethVector4 color)
{
    float clearColor[] = { color.x, color.y, color.z, color.w };
    m_CommandList->ClearRenderTargetView(texture.GetRTV(), clearColor, 0, nullptr);
}


void GraphicContext::ClearDepth(DepthStencilResource& depthTex, float val)
{
    m_CommandList->ClearDepthStencilView(depthTex.GetDSV(), D3D12_CLEAR_FLAG_DEPTH, val, 0, 0, nullptr);
}

void GraphicContext::TransitionResource(GpuResource& target, D3D12_RESOURCE_STATES newState)
{
    if (target.GetCurrentState() == newState)
        return;

    D3D12_RESOURCE_BARRIER barrier;
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = &target.GetResource();
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barrier.Transition.StateBefore = target.GetCurrentState();
    barrier.Transition.StateAfter = newState;
    m_CommandList->ResourceBarrier(1, &barrier);
    target.TransitionToState(newState);
}

void GraphicContext::SetRenderTarget(D3D12_CPU_DESCRIPTOR_HANDLE rtv)
{
    m_CommandList->OMSetRenderTargets(1, &rtv, FALSE, nullptr);
}

void GraphicContext::FinalizeAndExecute(bool waitForCompletion)
{
    GraphicCore::GetCommandManager().Execute(m_CommandList.Get());
    GraphicCore::GetCommandManager().DiscardAllocator(m_Type, *m_CommandAllocator);

    if (waitForCompletion)
        GetCommandQueue().Flush();
}

void GraphicContext::InitializeBuffer(BufferResource& dest, const void* data, size_t size, size_t dstOffset)
{
    GraphicContext context;
    GpuAllocation alloc = context.m_GpuAllocator.Allocate(size);
    memcpy(alloc.GetCpuAddress(), data, size);

    context.TransitionResource(dest, D3D12_RESOURCE_STATE_COPY_DEST);
    context.m_CommandList->CopyBufferRegion(&dest.GetResource(), dstOffset, &alloc.GetResource(), 0, size);
    context.TransitionResource(dest, D3D12_RESOURCE_STATE_GENERIC_READ);
    context.FinalizeAndExecute(true);
}

ETH_NAMESPACE_END

