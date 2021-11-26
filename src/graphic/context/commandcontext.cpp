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

#include "commandcontext.h"

ETH_NAMESPACE_BEGIN

CommandContext::CommandContext(D3D12_COMMAND_LIST_TYPE type)
    : m_Type(type)
{
    GraphicCore::GetCommandManager().CreateCommandList(type, &m_CommandList, &m_CommandAllocator);
}

CommandContext::~CommandContext()
{
}

void CommandContext::Reset()
{
    m_CommandAllocator = &GraphicCore::GetCommandManager().RequestAllocator(m_Type);
    m_CommandList->Reset(m_CommandAllocator, nullptr);
}

CommandQueue& CommandContext::GetCommandQueue() const
{
    return GraphicCore::GetCommandManager().GetQueue(m_Type);
}

uint64_t CommandContext::GetCompletionFenceValue() const
{
    return GetCommandQueue().GetCompletionFenceValue();
}

void CommandContext::TransitionResource(GpuResource& target, D3D12_RESOURCE_STATES newState)
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

void CommandContext::CopyBufferRegion(GpuResource& dest, GpuResource& src, size_t size, size_t dstOffset)
{
    TransitionResource(dest, D3D12_RESOURCE_STATE_COPY_DEST);
    m_CommandList->CopyBufferRegion(&dest.GetResource(), dstOffset, &src.GetResource(), 0, size);
}

void CommandContext::FinalizeAndExecute(bool waitForCompletion)
{
    GraphicCore::GetCommandManager().Execute(m_CommandList.Get());
    GraphicCore::GetCommandManager().DiscardAllocator(m_Type, *m_CommandAllocator);

    if (waitForCompletion)
        GetCommandQueue().Flush();
}

void CommandContext::InitializeBuffer(GpuResource& dest, const void* data, size_t size, size_t dstOffset)
{
    CommandContext context;
    GpuAllocation alloc = context.m_GpuAllocator.Allocate(size);
    memcpy(alloc.GetCpuAddress(), data, size);

    context.CopyBufferRegion(dest, alloc.GetResource(), size, dstOffset);
    context.TransitionResource(dest, D3D12_RESOURCE_STATE_GENERIC_READ);
    context.FinalizeAndExecute(true);
}

ETH_NAMESPACE_END

