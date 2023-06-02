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

#include "graphics/resources/material.h"

constexpr uint32_t MaterialVersion = 0;

Ether::Graphics::Material::Material()
    : Serializable(MaterialVersion, StringID("Graphics::Material").GetHash())
{
}

void Ether::Graphics::Material::Serialize(OStream& ostream) const
{
    Serializable::Serialize(ostream);
    ostream << m_BaseColor;
    ostream << m_SpecularColor;
    ostream << m_Roughness;
    ostream << m_Metalness;
}

void Ether::Graphics::Material::Deserialize(IStream& istream)
{
    Serializable::Deserialize(istream);
    istream >> m_BaseColor;
    istream >> m_SpecularColor;
    istream >> m_Roughness;
    istream >> m_Metalness;
}
