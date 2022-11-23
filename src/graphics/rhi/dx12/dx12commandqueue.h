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

#include "graphics/pch.h"
#include "graphics/rhi/rhicommandqueue.h"
#include "graphics/rhi/dx12/dx12includes.h"

namespace Ether::Graphics
{
    class Dx12CommandQueue : public RhiCommandQueue
    {
    public:
        Dx12CommandQueue(RhiCommandType type = RhiCommandType::Graphic);
        ~Dx12CommandQueue() = default;

    public:
        RhiFenceValue GetFinalFenceValue() const override;
        RhiFenceValue GetCurrentFenceValue() override;

        bool IsFenceComplete(RhiFenceValue fenceValue) override;
        void StallForFence(RhiFenceValue fenceValue) override;
        void Flush() override;
        void Execute(RhiCommandList& cmdList) override;

    private:
        friend class Dx12Device;
        wrl::ComPtr<ID3D12CommandQueue> m_CommandQueue;

        std::unique_ptr<RhiFence> m_Fence;
        RhiFenceValue m_FinalFenceValue;
        RhiFenceValue m_LastKnownFenceValue;

        void* m_FenceEventHandle;
    };
}

