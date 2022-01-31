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

#include "commandcontext.h"
#include "graphic/rhi/rhicommandlist.h"
#include "graphic/rhi/rhicommandqueue.h"
#include "graphic/rhi/rhiresource.h"

ETH_NAMESPACE_BEGIN

CommandContext::CommandContext(RHICommandListType type, const std::wstring& contextName)
    : m_Type(type)
    , m_ContextName(contextName)
{
    m_CommandList.SetName((m_ContextName + L"::CommandList").c_str());
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
    transitionDesc.m_Resource = target;
    transitionDesc.m_FromState = target->GetCurrentState();
    transitionDesc.m_ToState = newState;
    m_CommandList->TransitionResource(transitionDesc);
    target->SetState(newState);
}

void CommandContext::CopyBufferRegion(RHIResourceHandle src, RHIResourceHandle dest, size_t size, size_t destOffset)
{
    RHICopyBufferRegionDesc desc = {};
    desc.m_Source = src;
    desc.m_SourceOffset = 0;
    desc.m_Destination = dest;
    desc.m_DestinationOffset = destOffset;
    desc.m_Size = size;

    TransitionResource(dest, RHIResourceState::CopyDest);
    m_CommandList->CopyBufferRegion(desc);
}

void CommandContext::CopyTextureRegion(RHIResourceHandle src, RHIResourceHandle dest, const CompiledTexture& texture)
{
    RHICopyTextureRegionDesc desc = {};
    desc.m_Source = src;
    desc.m_Destination = dest;
    desc.m_Width = texture.GetWidth();
    desc.m_Height = texture.GetHeight();
    desc.m_Depth = 0;
    desc.m_BytesPerPixel = texture.GetBytesPerPixel();
    desc.m_Data = texture.GetData();

    TransitionResource(dest, RHIResourceState::CopyDest);
    m_CommandList->CopyTextureRegion(desc);
}

void CommandContext::InitializeBufferRegion(RHIResourceHandle dest, const void* data, size_t size, size_t destOffset)
{
    UploadBufferAllocation alloc = m_UploadBufferAllocator.Allocate(size);
    alloc.SetBufferData(data, size);

    CopyBufferRegion(alloc.GetUploadBuffer().GetResource(), dest, size, destOffset);
    TransitionResource(dest, RHIResourceState::GenericRead);
}

void CommandContext::InitializeTexture(const CompiledTexture& texture)
{
    size_t size = texture.GetSizeInBytes();
    UploadBufferAllocation alloc = m_UploadBufferAllocator.Allocate(size);

    CopyTextureRegion(alloc.GetUploadBuffer().GetResource(), texture.GetResource(), texture);
    TransitionResource(texture.GetResource(), RHIResourceState::GenericRead);
}

void CommandContext::FinalizeAndExecute(bool waitForCompletion)
{
    GraphicCore::GetCommandManager().Execute(m_CommandList);

    CommandAllocatorPool& allocatorPool = GraphicCore::GetCommandManager().GetAllocatorPool(m_Type);
    allocatorPool.DiscardAllocator(m_CommandAllocator, m_CommandQueue->GetCompletionFenceValue());

    if (waitForCompletion)
        m_CommandQueue->Flush();
}

void CommandContext::InitializeBufferTemp(RHIResourceHandle dest, const void* data, size_t size, size_t destOffset)
{
    CommandContext context(RHICommandListType::Graphic, L"BufferUploadContext");
    context.InitializeBufferRegion(dest, data, size, destOffset);
    context.FinalizeAndExecute(true);
}

ETH_NAMESPACE_END

