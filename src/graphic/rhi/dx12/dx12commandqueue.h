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

#pragma once

#include "graphic/rhi/rhicommandqueue.h"

ETH_NAMESPACE_BEGIN

class Dx12CommandQueue : public RhiCommandQueue
{
public:
    Dx12CommandQueue(RhiCommandListType type = RhiCommandListType::Graphic);
    ~Dx12CommandQueue() override;

public:
    inline RhiFenceValue GetCompletionFenceValue() const override;
    inline RhiFenceValue GetCompletedFenceValue() override;

    bool IsFenceComplete(RhiFenceValue fenceValue) override;

    RhiResult StallForFence(RhiFenceValue fenceValue) override;
    RhiResult Flush() override;
    RhiResult Execute(RhiCommandListHandle cmdList) override;

private:
    friend class Dx12Device;
    wrl::ComPtr<ID3D12CommandQueue> m_CommandQueue;

    RhiFenceHandle m_Fence;
    RhiFenceValue m_CompletionFenceValue;
    RhiFenceValue m_LastKnownFenceValue;

    void* m_FenceEventHandle;
};

ETH_NAMESPACE_END
