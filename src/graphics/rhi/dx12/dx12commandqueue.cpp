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

#ifdef ETH_GRAPHICS_DX12

#include "graphics/core.h"
#include "graphics/rhi/dx12/dx12commandqueue.h"
#include "graphics/rhi/dx12/dx12commandlist.h"
#include "graphics/rhi/dx12/dx12device.h"
#include "graphics/rhi/dx12/dx12fence.h"

Ether::Graphics::Dx12CommandQueue::Dx12CommandQueue(RhiCommandType type)
    : RhiCommandQueue(type)
    , m_FinalFenceValue(0)
    , m_LastKnownFenceValue(0)
{
    m_Fence = Core::GetDevice().CreateFence();
    m_FenceEventHandle = CreateEvent(nullptr, false, false, nullptr);
}

Ether::Graphics::RhiFenceValue Ether::Graphics::Dx12CommandQueue::GetFinalFenceValue() const
{ 
    return m_FinalFenceValue; 
}

Ether::Graphics::RhiFenceValue Ether::Graphics::Dx12CommandQueue::GetCurrentFenceValue()
{ 
    return m_Fence->GetCompletedValue();
}

bool Ether::Graphics::Dx12CommandQueue::IsFenceComplete(RhiFenceValue fenceValue)
{
    if (fenceValue > m_LastKnownFenceValue)
        m_LastKnownFenceValue = m_Fence->GetCompletedValue();

    return fenceValue <= m_LastKnownFenceValue;
}

void Ether::Graphics::Dx12CommandQueue::StallForFence(RhiFenceValue fenceValue)
{
    if (!IsFenceComplete(fenceValue))
    {
        m_Fence->SetEventOnCompletion(fenceValue, m_FenceEventHandle);
        WaitForSingleObject(m_FenceEventHandle, INFINITE);
    }
}

void Ether::Graphics::Dx12CommandQueue::Flush()
{
    const auto dx12Fence = dynamic_cast<Dx12Fence*>(m_Fence.get());
    HRESULT hr = m_CommandQueue->Signal(dx12Fence->m_Fence.Get(), ++m_FinalFenceValue);

    if (FAILED(hr))
        LogGraphicsError("Failed to signal command queue with fence");

    StallForFence(m_FinalFenceValue);
}

void Ether::Graphics::Dx12CommandQueue::Execute(RhiCommandList& cmdList)
{
    cmdList.Close();
    const auto dx12CmdList = dynamic_cast<Dx12CommandList*>(&cmdList);
    const auto dx12Fence = dynamic_cast<Dx12Fence*>(m_Fence.get());
    const auto graphicCmdList = dynamic_cast<ID3D12CommandList*>(dx12CmdList->m_CommandList.Get());
    m_CommandQueue->ExecuteCommandLists(1, &graphicCmdList);

    HRESULT hr = m_CommandQueue->Signal(dx12Fence->m_Fence.Get(), ++m_FinalFenceValue);

    if (FAILED(hr))
        LogGraphicsError("Failed to signal command queue with fence");
}

#endif // ETH_GRAPHICS_DX12

