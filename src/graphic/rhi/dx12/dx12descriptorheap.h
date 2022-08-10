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

#include "graphic/rhi/rhidescriptorheap.h"

ETH_NAMESPACE_BEGIN

class Dx12DescriptorHeap : public RhiDescriptorHeap
{
public:
    Dx12DescriptorHeap() = default;
    ~Dx12DescriptorHeap() override = default;

public:
    RhiCpuHandle GetBaseCpuHandle() const override;
    RhiGpuHandle GetBaseGpuHandle() const override;

    RhiCpuHandle GetNextCpuHandle() const override;
    RhiGpuHandle GetNextGpuHandle() const override;

    uint32_t GetHandleIncrementSize() const override;

    RhiResult IncrementHandle() override;

private:
    friend class Dx12CommandList;
    friend class Dx12Device;
    // TODO: Remove after GUI-Rhi refactor
    friend class GuiRenderer;

    wrl::ComPtr<ID3D12DescriptorHeap> m_Heap;

    size_t m_Offset;
    size_t m_HandleIncrementSize;
};

ETH_NAMESPACE_END
