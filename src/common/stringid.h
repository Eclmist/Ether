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

#include <string>
#include "common/hash.h"

ETH_NAMESPACE_BEGIN

using sid_t = uint32_t;

#define INVALID_SID sid_t(0xFFFFFFFF)

class StringID
{
public:
    StringID(sid_t sid = INVALID_SID);
    StringID(const char* str);
    StringID(const std::string& str);
    StringID(const StringID& other);

    const StringID& operator=(const StringID& other);
    bool operator==(const StringID& other) const;
    bool operator!=(const StringID& other) const;

    inline sid_t GetHash() const { return m_Hash; }

private:
    static sid_t Hash(const char* str);

private:
    sid_t m_Hash;
};

// Make sure the size of StringID is no larger than its internal representation.
static_assert(sizeof(StringID) == sizeof(sid_t), "StringID size is incorrect, it should be 32bits.");

ETH_NAMESPACE_END

template <>
struct std::hash<Ether::StringID> {
    std::size_t operator()(const Ether::StringID& key) const
    {
        return (std::size_t)(key.GetHash());
    }
};

