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

ETH_NAMESPACE_BEGIN

class DescriptorAllocatorPage;

class DescriptorAllocation
{
public:
    DescriptorAllocation(
        RhiCpuHandle allocBaseHandle,
        uint32_t numDescriptors,
        uint32_t pageOffset,
        uint32_t descriptorSize,
        DescriptorAllocatorPage* parentPage);

    ~DescriptorAllocation();

public:
    inline bool IsValid() const { return m_AllocationBaseHandle.m_Ptr == 0; };
    inline DescriptorAllocatorPage* GetParentPage() const { return m_ParentPage; }
    inline uint32_t GetNumDescriptors() const { return m_NumDescriptors; }
    inline uint32_t GetPageOffset() const { return m_PageOffset; }

public:
    RhiCpuHandle GetDescriptorHandle(uint32_t offset = 0) const;

private:
    void Free();

private:
    RhiCpuHandle m_AllocationBaseHandle;
    uint32_t m_NumDescriptors;
    uint32_t m_PageOffset;
    uint32_t m_DescriptorSize;

    DescriptorAllocatorPage* m_ParentPage;
};

ETH_NAMESPACE_END

