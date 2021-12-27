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
#include "graphic/rhi/rhicommandlist.h"
#include "graphic/rhi/rhicommandqueue.h"
#include "graphic/rhi/rhiresource.h"

ETH_NAMESPACE_BEGIN

CommandContext::CommandContext(RHICommandListType type)
    : m_Type(type)
{
    GraphicCore::GetCommandManager().CreateCommandList(type, m_CommandList, m_CommandAllocator);
    m_CommandQueue = GraphicCore::GetCommandManager().GetQueue(m_Type);
}

CommandContext::~CommandContext()
{
    m_CommandList.Destroy();
}

void CommandContext::Reset()
{
    CommandAllocatorPool& allocatorPool = GraphicCore::GetCommandManager().GetAllocatorPool(m_Type);
    m_CommandAllocator = allocatorPool.RequestAllocator(m_CommandQueue->GetCompletedFenceValue());
    m_CommandList->Reset(m_CommandAllocator);
}

RHIFenceValue CommandContext::GetCompletionFenceValue() const
{
    return GetCommandQueue()->GetCompletionFenceValue();
}

void CommandContext::TransitionResource(RHIResourceHandle target, RHIResourceState newState)
{
    if (target->GetCurrentState() == newState)
        return;

    RHIResourceTransitionDesc transitionDesc = {};
    transitionDesc.m_FromState = target->GetCurrentState();
    transitionDesc.m_ToState = newState;
    m_CommandList->TransitionResource(transitionDesc);
    target->SetState(newState);
}

void CommandContext::CopyBufferRegion(RHIResourceHandle dest, RHIResourceHandle src, size_t size, size_t dstOffset)
{
    RHICopyBufferRegionDesc desc = {};
    desc.m_Source = src;
    desc.m_SourceOffset = 0;
    desc.m_Destination = dest;
    desc.m_DestinationOffset = dstOffset;
    desc.m_Size = size;

    TransitionResource(dest, RHIResourceState::CopyDest);
    m_CommandList->CopyBufferRegion(desc);
}

void CommandContext::FinalizeAndExecute(bool waitForCompletion)
{
    GraphicCore::GetCommandManager().Execute(m_CommandList);

    CommandAllocatorPool& allocatorPool = GraphicCore::GetCommandManager().GetAllocatorPool(m_Type);
    allocatorPool.DiscardAllocator(m_CommandAllocator, m_CommandQueue->GetCompletionFenceValue());

    if (waitForCompletion)
        m_CommandQueue->Flush();
}

void CommandContext::InitializeBuffer(RHIResourceHandle dest, const void* data, size_t size, size_t dstOffset)
{
    CommandContext context;
    UploadBufferAllocation alloc = context.m_UploadBufferAllocator.Allocate(size);
    alloc.SetBufferData(data, size);

    // TODO: Add proper place to upload data - the current implementation in bufferresource is incorrect.
    context.CopyBufferRegion(dest, alloc.GetUploadBuffer().GetResource(), size, dstOffset);
    context.TransitionResource(dest, RHIResourceState::GenericRead);
    context.FinalizeAndExecute(true);
}

ETH_NAMESPACE_END

