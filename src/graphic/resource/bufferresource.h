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

#include "gpuresource.h"

ETH_NAMESPACE_BEGIN

class BufferResource : public GpuResource
{
public:
    BufferResource(const std::wstring& name, size_t numElements, size_t elementSize, const void* data);
    ~BufferResource() = default;

public:
    inline size_t GetBufferSize() const { return m_BufferSize; }
    inline size_t GetNumElements() const { return m_NumElements; }
    inline size_t GetElementSize() const { return m_ElementSize; }
    inline const D3D12_CPU_DESCRIPTOR_HANDLE GetSRV() const { return m_SRVHandle; }
    inline const D3D12_CPU_DESCRIPTOR_HANDLE GetUAV() const { return m_UAVHandle; }

protected:
    size_t m_BufferSize;
    size_t m_NumElements;
    size_t m_ElementSize;
    
    D3D12_CPU_DESCRIPTOR_HANDLE m_UAVHandle;
    D3D12_CPU_DESCRIPTOR_HANDLE m_SRVHandle;
};

ETH_NAMESPACE_END