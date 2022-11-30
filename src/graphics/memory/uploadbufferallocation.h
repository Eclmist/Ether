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

#pragma once

#include "graphics/pch.h"
#include "common/memory/linearallocator.h"

namespace Ether::Graphics
{
    class UploadBufferAllocation : public LinearAllocation
    {
    public:
        UploadBufferAllocation(
            size_t offset,
            size_t size,
            void* mappedBaseAddr,
            RhiResource& resource
        );

    public:
        inline void* GetBaseCpuHandle() const override { return m_MappedBaseAddress; }

        inline RhiGpuAddress GetBaseGpuAddress() const { return m_Resource.GetGpuAddress(); }
        inline RhiGpuAddress GetGpuAddress() const { return GetBaseGpuAddress() + m_Offset; }
        inline RhiResource& GetResource() { return m_Resource; }

    protected:
        void* m_MappedBaseAddress;
        RhiResource& m_Resource;
    };
}

