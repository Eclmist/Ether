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

#include "graphics/rhi/dx12/dx12descriptorheap.h"
#include "graphics/rhi/dx12/dx12translation.h"

Ether::Graphics::RhiCpuHandle Ether::Graphics::Dx12DescriptorHeap::GetBaseCpuHandle() const
{
    RhiCpuHandle handle;
    handle.m_Ptr = m_Heap->GetCPUDescriptorHandleForHeapStart().ptr;
    return handle;
}

Ether::Graphics::RhiGpuHandle Ether::Graphics::Dx12DescriptorHeap::GetBaseGpuHandle() const
{
    RhiGpuHandle handle;
    handle.m_Ptr = m_Heap->GetGPUDescriptorHandleForHeapStart().ptr;
    return handle;
}

Ether::Graphics::RhiCpuHandle Ether::Graphics::Dx12DescriptorHeap::GetNextCpuHandle() const
{
    RhiCpuHandle handle = GetBaseCpuHandle();
    handle.m_Ptr += m_Offset;
    return handle;
}

Ether::Graphics::RhiGpuHandle Ether::Graphics::Dx12DescriptorHeap::GetNextGpuHandle() const
{
    RhiGpuHandle handle = GetBaseGpuHandle();
    handle.m_Ptr += m_Offset;
    return handle;
}

uint32_t Ether::Graphics::Dx12DescriptorHeap::GetHandleIncrementSize() const
{
    return m_HandleIncrementSize;
}

void Ether::Graphics::Dx12DescriptorHeap::IncrementHandle()
{
    m_Offset += m_HandleIncrementSize;
}


