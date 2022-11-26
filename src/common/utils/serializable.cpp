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

#include "common/utils/serializable.h"
#include "common/stream/filestream.h"
#include <format>

Ether::Serializable::Serializable()
    : m_Version(0)
    , m_ClassID(typeid(this).hash_code())
{
    m_Guid = std::format(
        "{:X}-{:X}-{:X}-{:X}",
        SMath::Random::UniformInt(),
        SMath::Random::UniformInt(),
        SMath::Random::UniformInt(),
        SMath::Random::UniformInt()
    );
}

void Ether::Serializable::Serialize(OStream& ostream)
{
    ostream << m_Version;
    ostream << m_ClassID;
}

void Ether::Serializable::Deserialize(IStream& istream)
{
    istream >> m_Version;
    istream >> m_ClassID;
}
