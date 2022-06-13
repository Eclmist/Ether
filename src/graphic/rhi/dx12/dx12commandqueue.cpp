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

#include "dx12commandqueue.h"
#include "dx12commandlist.h"
#include "dx12device.h"
#include "dx12fence.h"

ETH_NAMESPACE_BEGIN

Dx12CommandQueue::Dx12CommandQueue(RhiCommandListType type)
    : RhiCommandQueue(type)
    , m_CompletionFenceValue(0)
    , m_LastKnownFenceValue(0)
{
    GraphicCore::GetDevice()->CreateFence(m_Fence);
    m_FenceEventHandle = CreateEvent(nullptr, false, false, nullptr);
}

Dx12CommandQueue::~Dx12CommandQueue()
{
    m_Fence.Destroy();
}

RhiFenceValue Dx12CommandQueue::GetCompletionFenceValue() const
{ 
    return m_CompletionFenceValue; 
}

RhiFenceValue Dx12CommandQueue::GetCompletedFenceValue()
{ 
    return m_Fence->GetCompletedValue();
}

bool Dx12CommandQueue::IsFenceComplete(RhiFenceValue fenceValue)
{
    if (fenceValue > m_LastKnownFenceValue)
        m_LastKnownFenceValue = m_Fence->GetCompletedValue();

    return fenceValue <= m_LastKnownFenceValue;
}

RhiResult Dx12CommandQueue::StallForFence(RhiFenceValue fenceValue)
{
    if (!IsFenceComplete(fenceValue))
    {
        ASSERT_SUCCESS(m_Fence->SetEventOnCompletion(fenceValue, m_FenceEventHandle));
        WaitForSingleObject(m_FenceEventHandle, INFINITE);
    }

    return RhiResult::Success;
}

RhiResult Dx12CommandQueue::Flush()
{
    const auto d3d12Fence = m_Fence.As<Dx12Fence>();
    HRESULT hr = m_CommandQueue->Signal(d3d12Fence->m_Fence.Get(), ++m_CompletionFenceValue);
    StallForFence(m_CompletionFenceValue);
    return TO_RHI_RESULT(hr);
}

RhiResult Dx12CommandQueue::Execute(RhiCommandListHandle cmdList)
{
    ASSERT_SUCCESS(cmdList->Close());
    const auto d3d12CmdList = cmdList.As<Dx12CommandList>();
    const auto d3d12Fence = m_Fence.As<Dx12Fence>();

    const auto graphicCmdList = dynamic_cast<ID3D12CommandList*>(d3d12CmdList->m_CommandList.Get());
    m_CommandQueue->ExecuteCommandLists(1, &graphicCmdList);
    HRESULT hr = m_CommandQueue->Signal(d3d12Fence->m_Fence.Get(), ++m_CompletionFenceValue);
    return TO_RHI_RESULT(hr);
}

ETH_NAMESPACE_END
