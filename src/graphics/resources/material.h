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

#include "graphics/pch.h"

#define ETH_CLASS_ID_MATERIAL "Graphics::Material"

namespace Ether::Graphics
{
class ETH_GRAPHIC_DLL Material : public Serializable
{
public:
    Material();
    ~Material() = default;

public:
    void Serialize(OStream& ostream) const override;
    void Deserialize(IStream& istream) override;

public:
    inline ethVector4 GetBaseColor() const { return m_BaseColor; }
    inline ethVector4 GetSpecularColor() const { return m_SpecularColor; }
    inline StringID GetAlbedoTextureID() const { return m_AlbedoTextureID; }
    inline StringID GetSpecularTextureID() const { return m_SpecularTextureID; }
    inline StringID GetRoughnessTextureID() const { return m_RoughnessTextureID; }
    inline StringID GetMetalnessTextureID() const { return m_MetalnessTextureID; }

    inline void SetBaseColor(const ethVector4& color) { m_BaseColor = color; }
    inline void SetSpecularColor(const ethVector4& color) { m_SpecularColor = color; }
    inline void SetAlbedoTextureID(const StringID& id) { m_AlbedoTextureID = id; }
    inline void SetSpecularTextureID(const StringID& id) { m_SpecularTextureID = id; }
    inline void SetRoughnessTextureID(const StringID& id) { m_RoughnessTextureID = id; }
    inline void SetMetalnessTextureID(const StringID& id) { m_MetalnessTextureID = id; }

private:
    ethVector4 m_BaseColor;
    ethVector4 m_SpecularColor;

    StringID m_AlbedoTextureID;
    StringID m_SpecularTextureID;
    StringID m_RoughnessTextureID;
    StringID m_MetalnessTextureID;
};
} // namespace Ether::Graphics
