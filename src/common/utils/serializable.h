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

#include "common/common.h"
#include <unordered_map>

namespace Ether
{
class OStream;
class IStream;

/*
    Derive from this class to make other classes serializable
*/
class ETH_COMMON_DLL Serializable
{
public:
    Serializable(uint32_t version, const char* classID, const std::string& name = "");
    virtual ~Serializable() = 0;

    inline std::string GetGuid() const { return m_Guid; }
    inline std::string GetName() const { return m_Name; }
    inline void SetName(const std::string& name) { m_Name = name; }

    virtual void Serialize(OStream& ostream) const;
    virtual void Deserialize(IStream& istream);
   
public:
    static std::string DeserializeClassID(IStream& istream);

protected:
    uint32_t m_Version;
    std::string m_Guid;
    std::string m_ClassID;
    std::string m_Name;
};
} // namespace Ether
