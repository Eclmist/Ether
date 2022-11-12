/*
    This file is part of Ether, an open-source DirectX 12 renderer.

    Copyright (c) 2020-2022 Samuel Huang - All rights reserved.

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

ETH_NAMESPACE_BEGIN

class DescriptorAllocator;

class DescriptorAllocation
{
public:
    DescriptorAllocation(
        RhiCpuHandle allocBaseCpuHandle,
        RhiGpuHandle allocBaseGpuHandle,
        uint32_t numDescriptors,
        uint32_t descriptorSize,
        uint32_t indexInAllocator,
        DescriptorAllocator* parentAllocator
    );

    ~DescriptorAllocation();

public:
    inline uint32_t GetNumDescriptors() const { return m_NumDescriptors; }

public:
    uint32_t GetDescriptorIndex(uint32_t offset = 0) const;
    RhiCpuHandle GetCpuHandle(uint32_t offset = 0) const;
    RhiGpuHandle GetGpuHandle(uint32_t offset = 0) const;

private:
    RhiCpuHandle m_BaseCpuHandle;
    RhiGpuHandle m_BaseGpuHandle;

    uint32_t m_IndexInAllocator;
    uint32_t m_DescriptorSize;
    uint32_t m_NumDescriptors;

    DescriptorAllocator* m_Parent;
};

ETH_NAMESPACE_END

