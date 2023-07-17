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

#include "common/stream/filestream.h"
#include <stdexcept>
#include <sstream>

Ether::IFileStream::IFileStream(const std::string& path)
{
    m_File.open(path, std::ios::in | std::ios::binary | std::ios::ate);
    if (!m_File.is_open())
        return;

    m_IsOpen = true;
    m_FileSize = m_File.tellg();
    m_File.seekg(0, std::ios::beg);
}

Ether::IFileStream::~IFileStream()
{
    m_File.close();
    m_IsOpen = false;
}

Ether::IStream& Ether::IFileStream::operator>>(float& value)
{
    ReadBytes(&value, sizeof(value));
    return *this;
}

Ether::IStream& Ether::IFileStream::operator>>(int& value)
{
    ReadBytes(&value, sizeof(value));
    return *this;
}

Ether::IStream& Ether::IFileStream::operator>>(long& value)
{
    ReadBytes(&value, sizeof(value));
    return *this;
}

Ether::IStream& Ether::IFileStream::operator>>(char& value)
{
    ReadBytes(&value, sizeof(value));
    return *this;
}

Ether::IStream& Ether::IFileStream::operator>>(unsigned int& value)
{
    ReadBytes(&value, sizeof(value));
    return *this;
}

Ether::IStream& Ether::IFileStream::operator>>(unsigned long& value)
{
    ReadBytes(&value, sizeof(value));
    return *this;
}

Ether::IStream& Ether::IFileStream::operator>>(unsigned char& value)
{
    ReadBytes(&value, sizeof(value));
    return *this;
}

Ether::IStream& Ether::IFileStream::operator>>(std::string& value)
{
    std::getline(m_File, value, '\0');
    return *this;
}

Ether::IStream& Ether::IFileStream::operator>>(StringID& value)
{
    std::string s;
    *this >> s;
    value = s;
    return *this;
}

Ether::IStream& Ether::IFileStream::operator>>(bool& value)
{
    ReadBytes(&value, sizeof(value));
    return *this;
}

Ether::IStream& Ether::IFileStream::operator>>(ethVector2& value)
{
    ReadBytes(&value, sizeof(value));
    return *this;
}

Ether::IStream& Ether::IFileStream::operator>>(ethVector3& value)
{
    ReadBytes(&value, sizeof(value));
    return *this;
}

Ether::IStream& Ether::IFileStream::operator>>(ethVector4& value)
{
    ReadBytes(&value, sizeof(value));
    return *this;
}

void Ether::IFileStream::ReadBytes(void* dest, uint32_t numBytes)
{
    m_File.read(reinterpret_cast<char*>(dest), numBytes);
}

Ether::OFileStream::OFileStream(const std::string& path)
{
    m_File.open(path, std::ios::out | std::ios::binary);
    if (!m_File.is_open())
        throw std::runtime_error("Failed to open file");

    m_IsOpen = m_File.is_open();
}

Ether::OFileStream::~OFileStream()
{
    m_File.close();
    m_IsOpen = false;
}

Ether::OStream& Ether::OFileStream::operator<<(const float value)
{
    WriteBytes(&value, sizeof(value));
    return *this;
}

Ether::OStream& Ether::OFileStream::operator<<(const int value)
{
    WriteBytes(&value, sizeof(value));
    return *this;
}

Ether::OStream& Ether::OFileStream::operator<<(const long value)
{
    WriteBytes(&value, sizeof(value));
    return *this;
}

Ether::OStream& Ether::OFileStream::operator<<(const char value)
{
    WriteBytes(&value, sizeof(value));
    return *this;
}

Ether::OStream& Ether::OFileStream::operator<<(const unsigned int value)
{
    WriteBytes(&value, sizeof(value));
    return *this;
}

Ether::OStream& Ether::OFileStream::operator<<(const unsigned long value)
{
    WriteBytes(&value, sizeof(value));
    return *this;
}

Ether::OStream& Ether::OFileStream::operator<<(const unsigned char value)
{
    WriteBytes(&value, sizeof(value));
    return *this;
}

Ether::OStream& Ether::OFileStream::operator<<(const std::string& value)
{
    WriteBytes(value.c_str(), value.size() + 1); // plus null terminator
    return *this;
}

Ether::OStream& Ether::OFileStream::operator<<(const StringID& value)
{
    *this << value.GetString();
    return *this;
}

Ether::OStream& Ether::OFileStream::operator<<(const bool value)
{
    WriteBytes(&value, sizeof(value));
    return *this;
}

Ether::OStream& Ether::OFileStream::operator<<(const ethVector2& value)
{
    WriteBytes(&value, sizeof(value));
    return *this;
}

Ether::OStream& Ether::OFileStream::operator<<(const ethVector3& value)
{
    WriteBytes(&value, sizeof(value));
    return *this;
}

Ether::OStream& Ether::OFileStream::operator<<(const ethVector4& value)
{
    WriteBytes(&value, sizeof(value));
    return *this;
}

void Ether::OFileStream::WriteBytes(const void* src, uint32_t numBytes)
{
    m_File.write((char*)src, numBytes);
}

void Ether::OFileStream::ClearFile()
{
    m_File.clear();
}
