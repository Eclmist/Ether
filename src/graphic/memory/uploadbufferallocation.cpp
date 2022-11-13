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

#include "uploadbufferallocation.h"

ETH_NAMESPACE_BEGIN


UploadBufferAllocation::UploadBufferAllocation(UploadBufferPage& uploadBuffer)
    : m_UploadBufferRef(uploadBuffer)
{
}

void UploadBufferAllocation::SetBufferData(const void* data, size_t size)
{
    if (size > m_Size)
    {
        LogGraphicsError("Buffer data size exceeded allocation size");
        size = m_Size;
    }

    memcpy(m_MappedCpuAddress, data, size);
}

ETH_NAMESPACE_END

