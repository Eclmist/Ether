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

#pragma once

#include "graphic/rhi/rhidescriptorheap.h"

ETH_NAMESPACE_BEGIN

class D3D12DescriptorHeap : public RHIDescriptorHeap
{
public:
    D3D12DescriptorHeap() = default;
    ~D3D12DescriptorHeap() override = default;

    RHIDescriptorHandleCPU GetBaseHandleCPU() const override;
    RHIDescriptorHandleGPU GetBaseHandleGPU() const override;

    RHIDescriptorHandleCPU GetNextHandleCPU() override;

private:
    friend class D3D12Device;
    // TODO: Remove after GUI-RHI refactor
    friend class GuiRenderer;
    wrl::ComPtr<ID3D12DescriptorHeap> m_Heap;

    size_t m_Offset;
    size_t m_HandleIncrementSize;
};

ETH_NAMESPACE_END
