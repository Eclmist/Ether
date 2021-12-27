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

#include "uploadbufferallocation.h"

ETH_NAMESPACE_BEGIN

class UploadBufferPage
{
public:
    UploadBufferPage(size_t size = 2048);
    ~UploadBufferPage();

public:
    inline size_t GetSize() const { return m_Size; }
    inline size_t GetOffset() const { return m_Offset; }

    inline RHIResourceHandle GetResource() const { return m_Resource; }

public:
    bool HasSpace(size_t size, size_t alignment);
    UploadBufferAllocation Allocate(size_t size, size_t alignment);
    void Reset();

private:
    RHIResourceHandle m_Resource;

    size_t m_Size;
    size_t m_Offset;
    void* m_CpuAddress;
};

ETH_NAMESPACE_END

