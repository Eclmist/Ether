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
    IStream& operator>>(int& v) override final;
    IStream& operator>>(long& v) override final;
    IStream& operator>>(char& v) override final;
    IStream& operator>>(unsigned int& v) override final;
    IStream& operator>>(unsigned long& v) override final;
    IStream& operator>>(unsigned char& v) override final;
    IStream& operator>>(std::string& v) override final;
    IStream& operator>>(StringID& v) override final;
    IStream& operator>>(bool& v) override final;
    IStream& operator>>(ethVector2& v) override final;
    IStream& operator>>(ethVector3& v) override final;
    IStream& operator>>(ethVector4& v) override final;

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
    OStream& operator<<(const int v) override final;
    OStream& operator<<(const long v) override final;
    OStream& operator<<(const char v) override final;
    OStream& operator<<(const unsigned int v) override final;
    OStream& operator<<(const unsigned long v) override final;
    OStream& operator<<(const unsigned char v) override final;
    OStream& operator<<(const std::string& v) override final;
    OStream& operator<<(const StringID& v) override final;
    OStream& operator<<(const bool v) override final;
    OStream& operator<<(const ethVector2& v) override final;
    OStream& operator<<(const ethVector3& v) override final;
    OStream& operator<<(const ethVector4& v) override final;

    void WriteBytes(const void* src, uint32_t numBytes) override final;

private:
    char* m_StartPtr;
    char* m_CurrPtr;
};

} // namespace Ether
