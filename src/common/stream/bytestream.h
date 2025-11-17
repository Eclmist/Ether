/*
    This file is part of Ether, an open-source DirectX 12 renderer.

    Copyright (c) 2020-2030 Samuel Huang - All rights reserved.

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

#include "common/stream/stream.h"
#include "common/stream/filestream.h"

namespace Ether
{
class ETH_COMMON_DLL IByteStream : public IStream
{
public:
    IByteStream(size_t size);
    IByteStream(IFileStream& file);
    ~IByteStream();

    IStream& operator>>(float& v) override final;
    IStream& operator>>(int8_t& v) override final;
    IStream& operator>>(int32_t& v) override final;
    IStream& operator>>(int64_t& v) override final;
    IStream& operator>>(uint8_t& v) override final;
    IStream& operator>>(uint32_t& v) override final;
    IStream& operator>>(uint64_t& v) override final;
    IStream& operator>>(std::string& v) override final;
    IStream& operator>>(StringID& v) override final;
    IStream& operator>>(bool& v) override final;
    IStream& operator>>(ethVectorData2& v) override final;
    IStream& operator>>(ethVectorData3& v) override final;
    IStream& operator>>(ethVectorData4& v) override final;
    IStream& operator>>(ethMatrixData3& v) override final;
    IStream& operator>>(ethMatrixData4& v) override final;

    void ReadBytes(void* dest, uint32_t numBytes) override final;

private:
    char* m_StartPtr;
    const char* m_CurrPtr;
    size_t m_Size;
};

class ETH_COMMON_DLL OByteStream : public OStream
{
public:
    OByteStream();
    ~OByteStream();

    OStream& operator<<(const float v) override final;
    OStream& operator<<(const int8_t v) override final;
    OStream& operator<<(const int32_t v) override final;
    OStream& operator<<(const int64_t v) override final;
    OStream& operator<<(const uint8_t v) override final;
    OStream& operator<<(const uint32_t v) override final;
    OStream& operator<<(const uint64_t v) override final;
    OStream& operator<<(const std::string& v) override final;
    OStream& operator<<(const StringID& v) override final;
    OStream& operator<<(const bool v) override final;
    OStream& operator<<(const ethVectorData2& v) override final;
    OStream& operator<<(const ethVectorData3& v) override final;
    OStream& operator<<(const ethVectorData4& v) override final;
    OStream& operator<<(const ethMatrixData3& v) override final;
    OStream& operator<<(const ethMatrixData4& v) override final;

    void WriteBytes(const void* src, uint32_t numBytes) override final;

private:
    char* m_StartPtr;
    char* m_CurrPtr;
};

} // namespace Ether
