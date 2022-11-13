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

#include "material.h"

ETH_NAMESPACE_BEGIN

void Material::Serialize(OStream& ostream)
{
    ostream << m_Roughness;
    ostream << m_Metalness;
    ostream << m_BaseColor;
    ostream << m_SpecularColor;
    ostream << m_AlbedoTexturePath;
    ostream << m_SpecularTexturePath;
}

void Material::Deserialize(IStream& istream)
{
    istream >> m_Roughness;
    istream >> m_Metalness;
    istream >> m_BaseColor;
    istream >> m_SpecularColor;
    istream >> m_AlbedoTexturePath;
    istream >> m_SpecularTexturePath;
}

CompiledTexture* Material::GetTexture(const std::string& key) const
{
    if (m_Textures.find(key) != m_Textures.end())
        return m_Textures.at(key).get();

    return nullptr;
}

ethVector4 Material::GetColor(const std::string& key) const
{
    if (m_Colors.find(key) != m_Colors.end())
        return m_Colors.at(key);

    return { 1.0, 1.0, 1.0, 1.0 };
}

void Material::SetTexture(const std::string& key, std::shared_ptr<CompiledTexture> texture)
{
    if (m_Textures.find(key) == m_Textures.end())
        m_Textures[key] = texture;
}

void Material::SetColor(const std::string& key, ethVector4 color)
{
    m_Colors[key] = color;
}

ETH_NAMESPACE_END

