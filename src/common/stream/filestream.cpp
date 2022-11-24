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

Ether::IFileStream::IFileStream(const std::string& path)
    : m_Path(path)
{
    m_File.open(path, std::ios::in | std::ios::binary);
    if (!m_File.is_open())
        throw std::runtime_error("Failed to open file");

    m_IsOpen = m_File.is_open();
}

Ether::IFileStream::~IFileStream()
{
    m_File.close();
}

std::string Ether::IFileStream::GetPath()
{
    return m_Path;
}

Ether::IFileStream& Ether::IFileStream::operator>>(float& value)
{
    m_File.read(reinterpret_cast<char*>(&value), sizeof(float));
    return *this;
}

Ether::IFileStream& Ether::IFileStream::operator>>(int& value)
{
    m_File.read(reinterpret_cast<char*>(&value), sizeof(int));
    return *this;
}

Ether::IFileStream& Ether::IFileStream::operator>>(long& value)
{
    m_File.read(reinterpret_cast<char*>(&value), sizeof(long));
    return *this;
}

Ether::IFileStream& Ether::IFileStream::operator>>(char& value)
{
    m_File.read(reinterpret_cast<char*>(&value), sizeof(char));
    return *this;
}

Ether::IFileStream& Ether::IFileStream::operator>>(unsigned int& value)
{
    m_File.read(reinterpret_cast<char*>(&value), sizeof(unsigned int));
    return *this;
}

Ether::IFileStream& Ether::IFileStream::operator>>(unsigned long& value)
{
    m_File.read(reinterpret_cast<char*>(&value), sizeof(unsigned long));
    return *this;
}

Ether::IFileStream& Ether::IFileStream::operator>>(unsigned char& value)
{
    m_File.read(reinterpret_cast<char*>(&value), sizeof(unsigned char));
    return *this;
}

Ether::IFileStream& Ether::IFileStream::operator>>(std::string& value)
{
    value = "";
    char c;
    do {
        m_File.read(reinterpret_cast<char*>(&c), sizeof(char));
        if (c == 0)
            break;
        value += c;
    } while (true);
    return *this;
}

Ether::IFileStream& Ether::IFileStream::operator>>(bool& value)
{
    m_File.read(reinterpret_cast<char*>(&value), sizeof(bool));
    return *this;
}

Ether::IFileStream& Ether::IFileStream::operator>>(ethVector2& v)
{
    m_File.read(reinterpret_cast<char*>(&v.x), sizeof(float));
    m_File.read(reinterpret_cast<char*>(&v.y), sizeof(float));
    return *this;
}

Ether::IFileStream& Ether::IFileStream::operator>>(ethVector3& v)
{
    m_File.read(reinterpret_cast<char*>(&v.x), sizeof(float));
    m_File.read(reinterpret_cast<char*>(&v.y), sizeof(float));
    m_File.read(reinterpret_cast<char*>(&v.z), sizeof(float));
    return *this;
}

Ether::IFileStream& Ether::IFileStream::operator>>(ethVector4& v)
{
    m_File.read(reinterpret_cast<char*>(&v.x), sizeof(float));
    m_File.read(reinterpret_cast<char*>(&v.y), sizeof(float));
    m_File.read(reinterpret_cast<char*>(&v.z), sizeof(float));
    m_File.read(reinterpret_cast<char*>(&v.w), sizeof(float));
    return *this;
}

Ether::OFileStream::OFileStream(const std::string& path)
    : m_Path(path)
{
    m_File.open(path, std::ios::out | std::ios::binary);
    if (!m_File.is_open())
        throw std::runtime_error("Failed to open file");

    m_IsOpen = m_File.is_open();
}

Ether::OFileStream::~OFileStream()
{
    m_File.close();
}

std::string Ether::OFileStream::GetPath()
{
    return m_Path;
}

Ether::OFileStream& Ether::OFileStream::operator<<(const float value)
{
    m_File.write(reinterpret_cast<const char*>(&value), sizeof(float));
    return *this;
}

Ether::OFileStream& Ether::OFileStream::operator<<(const int value)
{
    m_File.write(reinterpret_cast<const char*>(&value), sizeof(int));
    return *this;
}

Ether::OFileStream& Ether::OFileStream::operator<<(const long value)
{
    m_File.write(reinterpret_cast<const char*>(&value), sizeof(long));
    return *this;
}

Ether::OFileStream& Ether::OFileStream::operator<<(const char value)
{
    m_File.write(reinterpret_cast<const char*>(&value), sizeof(char));
    return *this;
}

Ether::OFileStream& Ether::OFileStream::operator<<(const unsigned int value)
{
    m_File.write(reinterpret_cast<const char*>(&value), sizeof(unsigned int));
    return *this;
}

Ether::OFileStream& Ether::OFileStream::operator<<(const unsigned long value)
{
    m_File.write(reinterpret_cast<const char*>(&value), sizeof(unsigned long));
    return *this;
}

Ether::OFileStream& Ether::OFileStream::operator<<(const unsigned char value)
{
    m_File.write(reinterpret_cast<const char*>(&value), sizeof(unsigned char));
    return *this;
}

Ether::OFileStream& Ether::OFileStream::operator<<(const std::string& value)
{
    if (!value.empty())
        m_File.write(value.c_str(), value.size());
    static const char end = 0;
    m_File.write(&end, sizeof(end));
    return *this;
}

Ether::OFileStream& Ether::OFileStream::operator<<(const bool value)
{
    m_File.write(reinterpret_cast<const char*>(&value), sizeof(bool));
    return *this;
}

Ether::OFileStream& Ether::OFileStream::operator<<(const ethVector2& v)
{
    m_File.write(reinterpret_cast<const char*>(&v.x), sizeof(float));
    m_File.write(reinterpret_cast<const char*>(&v.y), sizeof(float));
    return *this;
}

Ether::OFileStream& Ether::OFileStream::operator<<(const ethVector3& v)
{
    m_File.write(reinterpret_cast<const char*>(&v.x), sizeof(float));
    m_File.write(reinterpret_cast<const char*>(&v.y), sizeof(float));
    m_File.write(reinterpret_cast<const char*>(&v.z), sizeof(float));
    return *this;
}

Ether::OFileStream& Ether::OFileStream::operator<<(const ethVector4& v)
{
    m_File.write(reinterpret_cast<const char*>(&v.x), sizeof(float));
    m_File.write(reinterpret_cast<const char*>(&v.y), sizeof(float));
    m_File.write(reinterpret_cast<const char*>(&v.z), sizeof(float));
    m_File.write(reinterpret_cast<const char*>(&v.w), sizeof(float));
    return *this;
}

