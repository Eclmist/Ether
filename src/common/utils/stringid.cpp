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

#include "common/utils/stringid.h"

std::unordered_map<Ether::sid_t, std::string> Ether::StringID::s_HashToStringMap;

// Fast CRC32
// https://create.stephan-brumme.com/crc32/#bitwise
constexpr unsigned int crc32_bitwise(const char* data, std::size_t length)
{
    constexpr unsigned int Polynomial = 0xEDB88320;
    unsigned int crc = 0;
    while (length--) {
        crc ^= *data++;

        crc = (crc >> 1) ^ (-int(crc & 1) & Polynomial);
        crc = (crc >> 1) ^ (-int(crc & 1) & Polynomial);
        crc = (crc >> 1) ^ (-int(crc & 1) & Polynomial);
        crc = (crc >> 1) ^ (-int(crc & 1) & Polynomial);
        crc = (crc >> 1) ^ (-int(crc & 1) & Polynomial);
        crc = (crc >> 1) ^ (-int(crc & 1) & Polynomial);
        crc = (crc >> 1) ^ (-int(crc & 1) & Polynomial);
        crc = (crc >> 1) ^ (-int(crc & 1) & Polynomial);
    }
    return ~crc;
}

Ether::StringID::StringID()
    : m_Hash(InvalidSid)
{
}

Ether::StringID::StringID(const char* str)
    : m_Hash(Hash(str))
{
    s_HashToStringMap[m_Hash] = str;
}

std::string Ether::StringID::GetString() const
{
    return s_HashToStringMap.find(m_Hash) == s_HashToStringMap.end()
        ? "Invalid StringID"
        : s_HashToStringMap.at(m_Hash);
}

Ether::StringID::StringID(const std::string& str)
    : m_Hash(Hash(str.c_str()))
{
    s_HashToStringMap[m_Hash] = str;
}

bool Ether::StringID::operator==(const StringID& other) const
{
    return m_Hash == other.m_Hash;
}

bool Ether::StringID::operator!=(const StringID& other) const
{
    return m_Hash != other.m_Hash;
}

Ether::sid_t Ether::StringID::Hash(const char* str)
{
    return crc32_bitwise(str, std::char_traits<char>::length(str));
}

