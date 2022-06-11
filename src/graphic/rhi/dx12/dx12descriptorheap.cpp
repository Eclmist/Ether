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

#include "dx12descriptorheap.h"
#include "dx12translation.h"

ETH_NAMESPACE_BEGIN

RhiCpuHandle Dx12DescriptorHeap::GetBaseHandleCpu() const
{
    RhiCpuHandle handle;
    handle.m_Ptr = m_Heap->GetCPUDescriptorHandleForHeapStart().ptr;
    return handle;
}

RhiGpuHandle Dx12DescriptorHeap::GetBaseGpuHandle() const
{
    RhiGpuHandle handle;
    handle.m_Ptr = m_Heap->GetGPUDescriptorHandleForHeapStart().ptr;
    return handle;
}

RhiCpuHandle Dx12DescriptorHeap::GetNextCpuHandle() const
{
    RhiCpuHandle handle = GetBaseHandleCpu();
    handle.m_Ptr += m_Offset;
    return handle;
}

RhiGpuHandle Dx12DescriptorHeap::GetNextGpuHandle() const
{
    RhiGpuHandle handle = GetBaseGpuHandle();
    handle.m_Ptr += m_Offset;
    return handle;
}

RhiResult Dx12DescriptorHeap::IncrementHandle()
{
    m_Offset += m_HandleIncrementSize;
    return RhiResult::Success;
}

ETH_NAMESPACE_END

