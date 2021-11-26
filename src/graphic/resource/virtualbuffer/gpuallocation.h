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

ETH_NAMESPACE_BEGIN

class GpuAllocation
{
public:
    GpuAllocation(GpuResource& baseResource);

    inline GpuResource& GetResource() const { return m_Resource; }
    inline void* GetCpuAddress() const { return m_CpuAddress; }
    inline D3D12_GPU_VIRTUAL_ADDRESS GetGpuAddress() const { return m_GpuAddress; }
    inline size_t GetSize() const { return m_Size; }
    inline size_t GetOffset() const { return m_Offset; }

private:
    friend class LinearAllocatorPage;
    inline void SetCpuAddress(void* cpuAddress) { m_CpuAddress = cpuAddress; }
    inline void SetGpuAddress(D3D12_GPU_VIRTUAL_ADDRESS gpuAddress) { m_GpuAddress = gpuAddress; }
    inline void SetSize(size_t size) { m_Size = size; }
    inline void SetOffset(size_t offset) { m_Offset = offset; }

private:
    GpuResource& m_Resource;

    void* m_CpuAddress;
    D3D12_GPU_VIRTUAL_ADDRESS m_GpuAddress;

    size_t m_Size;
    size_t m_Offset;
};

ETH_NAMESPACE_END
