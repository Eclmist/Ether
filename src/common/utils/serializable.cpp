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

Ether::Serializable::Serializable(uint32_t version, uint32_t classID)
    : m_Version(version)
    , m_ClassID(classID)
{
    m_Guid = std::format(
        "{:X}-{:X}-{:X}-{:X}",
        SMath::Random::UniformInt(),
        SMath::Random::UniformInt(),
        SMath::Random::UniformInt(),
        SMath::Random::UniformInt()
    );
}

Ether::Serializable::~Serializable()
{
}

void Ether::Serializable::Serialize(OStream& ostream)
{
    ostream << m_Version;
    ostream << m_ClassID;
}

void Ether::Serializable::Deserialize(IStream& istream)
{
    uint32_t version;
    istream >> version;
    if (m_Version != version)
        throw std::runtime_error(std::format("Asset version mismatch - expected version {} but found version {}", version, m_Version));

    istream >> m_ClassID;
}

