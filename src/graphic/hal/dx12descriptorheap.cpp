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

#include "dx12descriptorheap.h"

ETH_NAMESPACE_BEGIN

DX12DescriptorHeap::DX12DescriptorHeap(
wrl::ComPtr<ID3D12Device3> device,
    D3D12_DESCRIPTOR_HEAP_TYPE type,
    uint32_t numDescriptors,
    D3D12_DESCRIPTOR_HEAP_FLAGS flags)
    : m_Type(type)
    , m_NumDescriptors(numDescriptors)
{
    D3D12_DESCRIPTOR_HEAP_DESC desc = {};
    desc.NumDescriptors = m_NumDescriptors;
    desc.Type = m_Type;
    desc.Flags = flags;
    ThrowIfFailed(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_DescriptorHeap)));

    m_DescriptorHandleStride = device->GetDescriptorHandleIncrementSize(m_Type);
    m_BaseDescriptor = m_DescriptorHeap->GetCPUDescriptorHandleForHeapStart();
}

CD3DX12_CPU_DESCRIPTOR_HANDLE DX12DescriptorHeap::GetHandle(uint32_t offset) const
{
    return CD3DX12_CPU_DESCRIPTOR_HANDLE(m_BaseDescriptor, offset, m_DescriptorHandleStride);
}

uint32_t DX12DescriptorHeap::ComputeOffset(D3D12_CPU_DESCRIPTOR_HANDLE handle) const
{
    return static_cast<uint32_t>(handle.ptr - m_BaseDescriptor.ptr) / m_DescriptorHandleStride;
}

ETH_NAMESPACE_END
