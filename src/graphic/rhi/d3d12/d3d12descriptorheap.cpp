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

#include "d3d12descriptorheap.h"
#include "d3d12translation.h"

ETH_NAMESPACE_BEGIN

RHIDescriptorHandleCPU D3D12DescriptorHeap::GetBaseHandleCPU() const
{
    return Translate(m_Heap->GetCPUDescriptorHandleForHeapStart());
}

RHIDescriptorHandleGPU D3D12DescriptorHeap::GetBaseHandleGPU() const
{
    return Translate(m_Heap->GetGPUDescriptorHandleForHeapStart());
}

RHIDescriptorHandleCPU D3D12DescriptorHeap::GetNextHandleCPU()
{
    RHIDescriptorHandleCPU handle = GetBaseHandleCPU();
    handle.m_Ptr += m_Offset;
    m_Offset += m_HandleIncrementSize;

    return handle;
}

ETH_NAMESPACE_END

