/*
    This file is part of Ether, an open-source DirectX 12 renderer.

    Copyright (c) 2020-2021 Samuel Van Allen - All rights reserved.

    Ether is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

ETH_NAMESPACE_BEGIN

/*
    Derive from this class to make other classes serializable
*/

struct SerializableField
{
    void* m_Data;
    size_t m_Size;
};

class OStream;
class IStream;

class Serializable
{
public:
    Serializable();
    ~Serializable() = default;

    inline std::string GetGuid() const { return m_Guid; }

    void Serialize(OStream& ostream);
    void Deserialize(IStream& istream);

protected:
    //virtual void Serialize_Impl(OStream& ostream) = 0;
    //virtual void Deserialize_Impl(IStream& istream) = 0;

private:
    std::string m_Guid;
    uint32_t m_Version;
    uint32_t m_ClassID;

    std::unordered_map<std::string, SerializableField> m_Fields;
};

ETH_NAMESPACE_END

