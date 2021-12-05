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

#include "serializable.h"
#include "common/stream/filestream.h"

ETH_NAMESPACE_BEGIN

Serializable::Serializable()
    : m_ClassID(typeid(this).hash_code())
    , m_Version(1)
{
}

void Serializable::Serialize(OStream& ostream)
{
    ostream << m_Version;
    ostream << m_ClassID;
    Serialize_Impl(ostream);
}

void Serializable::Deserialize(IStream& istream)
{
    istream >> m_Version;
    istream >> m_ClassID;
    Deserialize_Impl(istream);
}

void Serializable::Serialize_Impl(OStream& ostream)
{
}

void Serializable::Deserialize_Impl(IStream& istream)
{
}

ETH_NAMESPACE_END

