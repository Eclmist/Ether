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

#include "commandqueue.h"

ETH_NAMESPACE_BEGIN

CommandQueue::CommandQueue(D3D12_COMMAND_LIST_TYPE type)
    : m_Type(type)
    , m_AllocatorPool(type)
{
    AssertGraphics(g_GraphicDevice != nullptr,
        "An attempt was made to create a command queue before initializing the graphics device");

    InitializeCommandQueue();
    InitializeFence();
}

uint64_t CommandQueue::Execute(ID3D12CommandList* cmdLst)
{
    ASSERT_SUCCESS(((ID3D12GraphicsCommandList*)cmdLst)->Close());
    m_CommandQueue->ExecuteCommandLists(1, &cmdLst);
    m_CommandQueue->Signal(m_Fence.Get(), ++m_CompletionFenceValue);
    return m_CompletionFenceValue;
}

bool CommandQueue::IsFenceComplete(uint64_t fenceValue)
{
    if (fenceValue > m_LastKnownFenceValue)
        m_LastKnownFenceValue = m_Fence->GetCompletedValue();

    return fenceValue <= m_LastKnownFenceValue;
}

void CommandQueue::StallForFence(uint64_t fenceValue)
{
    if (!IsFenceComplete(fenceValue))
    {
        ASSERT_SUCCESS(m_Fence->SetEventOnCompletion(fenceValue, m_FenceEventHandle));
        WaitForSingleObject(m_FenceEventHandle, INFINITE);
    }
}

void CommandQueue::Flush()
{
    StallForFence(m_CompletionFenceValue);
}

void CommandQueue::InitializeCommandQueue()
{
    D3D12_COMMAND_QUEUE_DESC desc = {};
    desc.Type = m_Type;
    desc.NodeMask = 0;
    desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

    ASSERT_SUCCESS(g_GraphicDevice->CreateCommandQueue(&desc, IID_PPV_ARGS(&m_CommandQueue)));
    m_CommandQueue->SetName(L"CommandQueue::m_CommandQueue");
}

void CommandQueue::InitializeFence()
{
    ASSERT_SUCCESS(g_GraphicDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_Fence)));
    m_Fence->SetName(L"CommandQueue::m_Fence");
    m_CompletionFenceValue = 0;
    m_LastKnownFenceValue = 0;
    m_FenceEventHandle = CreateEvent(nullptr, false, false, nullptr);
    AssertGraphics(m_FenceEventHandle != NULL, "Failed to create fence event");
}

ID3D12CommandAllocator* CommandQueue::RequestAllocator()
{
    return m_AllocatorPool.RequestAllocator(m_Fence->GetCompletedValue());
}

void CommandQueue::DiscardAllocator(ID3D12CommandAllocator* allocator, uint64_t fenceValue)
{
    m_AllocatorPool.DiscardAllocator(allocator, fenceValue);
}

ETH_NAMESPACE_END

