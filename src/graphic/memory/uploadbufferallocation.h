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

ETH_NAMESPACE_BEGIN

class UploadBufferPage; // TODO: Remove this when commandcontext::initializebuffer copy fixed

class UploadBufferAllocation
{
public:
    UploadBufferAllocation(UploadBufferPage& uploadBuffer);

    inline size_t GetSize() const { return m_Size; }
    inline size_t GetOffset() const { return m_Offset; }

    // TODO: Remove this when commandcontext::initializebuffer copy fixed
    inline UploadBufferPage& GetUploadBuffer() const { return m_UploadBufferRef; }

    void SetBufferData(const void* data, size_t size);

private:
    friend class UploadBufferPage;
    inline void SetMappedCpuAddress(void* cpuAddress) { m_MappedCpuAddress = cpuAddress; }
    inline void SetSize(size_t size) { m_Size = size; }
    inline void SetOffset(size_t offset) { m_Offset = offset; }

private:
    // TODO: Remove this when commandcontext::initializebuffer copy fixed
    UploadBufferPage& m_UploadBufferRef;
    void* m_MappedCpuAddress;

    size_t m_Size;
    size_t m_Offset;
};

ETH_NAMESPACE_END
