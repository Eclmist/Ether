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

#pragma once

#include "graphic/rhi/rhicommandqueue.h"

ETH_NAMESPACE_BEGIN

class D3D12CommandQueue : public RHICommandQueue
{
public:
    D3D12CommandQueue(RHICommandListType type = RHICommandListType::Graphic);
    ~D3D12CommandQueue() override;

public:
    inline RHIFenceValue GetCompletionFenceValue() const override;
    inline RHIFenceValue GetCompletedFenceValue() override;

    bool IsFenceComplete(RHIFenceValue fenceValue) override;

    RHIResult StallForFence(RHIFenceValue fenceValue) override;
    RHIResult Flush() override;
    RHIResult Execute(RHICommandListHandle cmdList) override;

private:
    friend class D3D12Device;
    wrl::ComPtr<ID3D12CommandQueue> m_CommandQueue;

    RHIFenceHandle m_Fence;
    RHIFenceValue m_CompletionFenceValue;
    RHIFenceValue m_LastKnownFenceValue;

    HANDLE m_FenceEventHandle;
};

ETH_NAMESPACE_END
