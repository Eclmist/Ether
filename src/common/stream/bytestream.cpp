/*
    This file is part of Ether, an open-source DirectX 12 renderer.

    Copyright (c) 2020-2021 Samuel valuean Allen - All rights reserved.

    Ether is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either valueersion 3 of the License, or
    (at your option) any later valueersion.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "common/stream/bytestream.h"
#include <stdexcept>

Ether::IByteStream::IByteStream(size_t size)
    : m_Size(size)
{
    m_StartPtr = new char[size];
    m_CurrPtr = m_StartPtr;
    m_IsOpen = true;
}

Ether::IByteStream::IByteStream(IFileStream& file)
{
    m_StartPtr = new char[file.GetFileSize()];
    m_CurrPtr = m_StartPtr;
    file.ReadBytes(m_StartPtr, file.GetFileSize());
    m_IsOpen = true;
}

Ether::IByteStream::~IByteStream()
{
    m_IsOpen = false;
    free(m_StartPtr);
}

Ether::IStream& Ether::IByteStream::operator>>(float& value)
{
    ReadBytes(&value, sizeof(value));
    return *this;
}

Ether::IStream& Ether::IByteStream::operator>>(int& value)
{
    ReadBytes(&value, sizeof(value));
    return *this;
}

Ether::IStream& Ether::IByteStream::operator>>(long& value)
{
    ReadBytes(&value, sizeof(value));
    return *this;
}

Ether::IStream& Ether::IByteStream::operator>>(char& value)
{
    ReadBytes(&value, sizeof(value));
    return *this;
}

Ether::IStream& Ether::IByteStream::operator>>(unsigned int& value)
{
    ReadBytes(&value, sizeof(value));
    return *this;
}

Ether::IStream& Ether::IByteStream::operator>>(unsigned long& value)
{
    ReadBytes(&value, sizeof(value));
    return *this;
}

Ether::IStream& Ether::IByteStream::operator>>(unsigned char& value)
{
    ReadBytes(&value, sizeof(value));
    return *this;
}

Ether::IStream& Ether::IByteStream::operator>>(std::string& value)
{
    value = m_CurrPtr;             // Construct string from the pointer directly since it is null terminated
    m_CurrPtr += value.size() + 1; // +1 for null terminator
    return *this;
}

Ether::IStream& Ether::IByteStream::operator>>(StringID& value)
{
    std::string s;
    *this >> s;
    value = s;
    return *this;
}

Ether::IStream& Ether::IByteStream::operator>>(bool& value)
{
    ReadBytes(&value, sizeof(value));
    return *this;
}

Ether::IStream& Ether::IByteStream::operator>>(ethVector2& value)
{
    ReadBytes(&value, sizeof(value));
    return *this;
}

Ether::IStream& Ether::IByteStream::operator>>(ethVector3& value)
{
    ReadBytes(&value, sizeof(value));
    return *this;
}

Ether::IStream& Ether::IByteStream::operator>>(ethVector4& value)
{
    ReadBytes(&value, sizeof(value));
    return *this;
}

void Ether::IByteStream::ReadBytes(void* dest, uint32_t numBytes)
{
    memcpy(reinterpret_cast<char*>(dest), m_CurrPtr, numBytes);
    m_CurrPtr += numBytes;
}

Ether::OByteStream::OByteStream()
{
    m_IsOpen = true;
    m_StartPtr = new char[1000000000];
    m_CurrPtr = m_StartPtr;
}

Ether::OByteStream::~OByteStream()
{
    m_IsOpen = false;
    free(m_StartPtr);
}

Ether::OStream& Ether::OByteStream::operator<<(const float value)
{
    WriteBytes(&value, sizeof(value));
    return *this;
}

Ether::OStream& Ether::OByteStream::operator<<(const int value)
{
    WriteBytes(&value, sizeof(value));
    return *this;
}

Ether::OStream& Ether::OByteStream::operator<<(const long value)
{
    WriteBytes(&value, sizeof(value));
    return *this;
}

Ether::OStream& Ether::OByteStream::operator<<(const char value)
{
    WriteBytes(&value, sizeof(value));
    return *this;
}

Ether::OStream& Ether::OByteStream::operator<<(const unsigned int value)
{
    WriteBytes(&value, sizeof(value));
    return *this;
}

Ether::OStream& Ether::OByteStream::operator<<(const unsigned long value)
{
    WriteBytes(&value, sizeof(value));
    return *this;
}

Ether::OStream& Ether::OByteStream::operator<<(const unsigned char value)
{
    WriteBytes(&value, sizeof(value));
    return *this;
}

Ether::OStream& Ether::OByteStream::operator<<(const std::string& value)
{
    WriteBytes(value.c_str(), value.size() + 1); // plus null terminator
    return *this;
}

Ether::OStream& Ether::OByteStream::operator<<(const StringID& value)
{
    *this << value.GetString();
    return *this;
}

Ether::OStream& Ether::OByteStream::operator<<(const bool value)
{
    WriteBytes(&value, sizeof(value));
    return *this;
}

Ether::OStream& Ether::OByteStream::operator<<(const ethVector2& value)
{
    WriteBytes(&value, sizeof(value));
    return *this;
}

Ether::OStream& Ether::OByteStream::operator<<(const ethVector3& value)
{
    WriteBytes(&value, sizeof(value));
    return *this;
}

Ether::OStream& Ether::OByteStream::operator<<(const ethVector4& value)
{
    WriteBytes(&value, sizeof(value));
    return *this;
}

void Ether::OByteStream::WriteBytes(const void* src, uint32_t numBytes)
{
    memcpy(m_CurrPtr, src, numBytes);
}
