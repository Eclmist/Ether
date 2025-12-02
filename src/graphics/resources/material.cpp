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

#include "graphics/resources/material.h"

constexpr uint32_t MaterialVersion = 1;

Ether::Graphics::Material::Material()
    : Serializable(MaterialVersion, ETH_CLASS_ID_MATERIAL)
    , m_BaseColor(1.0f, 1.0f, 1.0f)
    , m_EmissiveColor(0.0f, 0.0f, 0.0f)
    , m_Roughness(0.5f)
    , m_Metalness(0.0f)
    , m_Opacity(1.0f)
    , m_BaseColorTextureID()
    , m_NormalTextureID()
    , m_MetalnessTextureID()
    , m_RoughnessTextureID()
    , m_EmissiveTextureID()
{
}

void Ether::Graphics::Material::Serialize(OStream& ostream) const
{
    Serializable::Serialize(ostream);
    ostream << m_BaseColor;
    ostream << m_EmissiveColor;
    ostream << m_Roughness;
    ostream << m_Metalness;
    ostream << m_Opacity;
    ostream << m_BaseColorTextureID;
    ostream << m_NormalTextureID;
    ostream << m_MetalnessTextureID;
    ostream << m_RoughnessTextureID;
    ostream << m_EmissiveTextureID;

    ostream << static_cast<uint32_t>(m_BlendMode);
    ostream << static_cast<uint32_t>(m_RaytracingVisibility);
}

void Ether::Graphics::Material::Deserialize(IStream& istream)
{
    Serializable::Deserialize(istream);
    istream >> m_BaseColor;
    istream >> m_EmissiveColor;
    istream >> m_Roughness;
    istream >> m_Metalness;
    istream >> m_Opacity;
    istream >> m_BaseColorTextureID;
    istream >> m_NormalTextureID;
    istream >> m_MetalnessTextureID;
    istream >> m_RoughnessTextureID;
    istream >> m_EmissiveTextureID;

    uint32_t blendMode;
    istream >> blendMode;
    m_BlendMode = static_cast<BlendMode>(blendMode);

    uint32_t raytracingVisibility;
    istream >> raytracingVisibility;
    m_RaytracingVisibility = static_cast<RaytracingVisibility>(raytracingVisibility);
}

bool Ether::Graphics::Material::HasTranslucency() const
{
    return m_BlendMode == BlendMode::Translucent || m_BlendMode == BlendMode::Additive;
}
