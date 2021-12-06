/*
    This file is part of Ether, an open-source DirectX 12 renderer.

    Copyright (c) 2020-2021 Samuel valuean Allen - All rights reserved.

    Ether is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either valueersion 3 of the License, or
    (at your option) any later valueersion.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "filestream.h"

ETH_NAMESPACE_BEGIN

IFileStream::IFileStream(const std::string& path)
    : m_Path(path)
{
    m_File.open(path, std::ios::in | std::ios::binary);
    if (!m_File.is_open())
        LogEngineError("Failed to open file %s", path.c_str());
}

IFileStream::~IFileStream()
{
    m_File.close();
}

std::string IFileStream::GetPath()
{
    return m_Path;
}

IFileStream& IFileStream::operator>>(float& value)
{
    m_File.read(reinterpret_cast<char*>(&value), sizeof(float));
    return *this;
}

IFileStream& IFileStream::operator>>(int& value)
{
    m_File.read(reinterpret_cast<char*>(&value), sizeof(int));
    return *this;
}

IFileStream& IFileStream::operator>>(long& value)
{
    m_File.read(reinterpret_cast<char*>(&value), sizeof(long));
    return *this;
}

IFileStream& IFileStream::operator>>(char& value)
{
    m_File.read(reinterpret_cast<char*>(&value), sizeof(char));
    return *this;
}

IFileStream& IFileStream::operator>>(unsigned int& value)
{
    m_File.read(reinterpret_cast<char*>(&value), sizeof(unsigned int));
    return *this;
}

IFileStream& IFileStream::operator>>(unsigned long& value)
{
    m_File.read(reinterpret_cast<char*>(&value), sizeof(unsigned long));
    return *this;
}

IFileStream& IFileStream::operator>>(std::string& value)
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

IFileStream& IFileStream::operator>>(bool& value)
{
    m_File.read(reinterpret_cast<char*>(&value), sizeof(bool));
    return *this;
}


OFileStream::OFileStream(const std::string& path)
    : m_Path(path)
{
    m_File.open(path, std::ios::out | std::ios::binary);
    if (!m_File.is_open())
        LogEngineError("Failed to open file %s", path.c_str());
}

OFileStream::~OFileStream()
{
    m_File.close();
}

std::string OFileStream::GetPath()
{
    return m_Path;
}

OFileStream& OFileStream::operator<<(const float value)
{
    m_File.write(reinterpret_cast<const char*>(&value), sizeof(float));
    return *this;
}

OFileStream& OFileStream::operator<<(const int value)
{
    m_File.write(reinterpret_cast<const char*>(&value), sizeof(int));
    return *this;
}

OFileStream& OFileStream::operator<<(const long value)
{
    m_File.write(reinterpret_cast<const char*>(&value), sizeof(long));
    return *this;
}

OFileStream& OFileStream::operator<<(const char value)
{
    m_File.write(reinterpret_cast<const char*>(&value), sizeof(char));
    return *this;
}

OFileStream& OFileStream::operator<<(const unsigned int value)
{
    m_File.write(reinterpret_cast<const char*>(&value), sizeof(unsigned int));
    return *this;
}

OFileStream& OFileStream::operator<<(const unsigned long value)
{
    m_File.write(reinterpret_cast<const char*>(&value), sizeof(unsigned long));
    return *this;
}

OFileStream& OFileStream::operator<<(const std::string& value)
{
    if (!value.empty())
        m_File.write(value.c_str(), value.size());
    static const char end = 0;
    m_File.write(&end, sizeof(end));
    return *this;
}

OFileStream& OFileStream::operator<<(const bool value)
{
    m_File.write(reinterpret_cast<const char*>(&value), sizeof(bool));
    return *this;
}

ETH_NAMESPACE_END

