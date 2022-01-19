/*
    This file is part of Ether, an open-source DirectX 12 renderer.

    Copyright (c) 2020-2022 Samuel Huang - All rights reserved.

    Ether is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "stringid.h"

ETH_NAMESPACE_BEGIN

StringID::StringID(sid_t sid)
    : m_Hash(sid)
{
}

StringID::StringID(const char* str)
    : m_Hash(Hash(str))
{
}

StringID::StringID(const std::string& str)
    : m_Hash(Hash(str.c_str()))
{
}

StringID::StringID(const StringID& other)
    : m_Hash(other.m_Hash)
{
}

const StringID& StringID::operator=(const StringID& other)
{
    m_Hash = other.m_Hash;
    return *this;
}

bool StringID::operator==(const StringID& other) const
{
    return m_Hash == other.m_Hash;
}

bool StringID::operator!=(const StringID& other) const
{
    return m_Hash != other.m_Hash;
}

sid_t StringID::Hash(const char* str)
{
    return crc32_bitwise(str, std::char_traits<char>::length(str));
}

ETH_NAMESPACE_END

