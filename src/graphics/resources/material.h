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
    inline uint32_t GetTransientMaterialIdx() const { return m_TransientMaterialIdx; }
    inline ethVector3 GetBaseColor() const { return m_BaseColor; }
    inline ethVector3 GetEmissiveColor() const { return m_EmissiveColor; }
    inline float GetRoughness() const { return m_Roughness; }
    inline float GetMetalness() const { return m_Metalness; }
    inline float GetOpacity() const { return m_Opacity; }
    inline StringID GetBaseColorTextureID() const { return m_BaseColorTextureID; }
    inline StringID GetNormalTextureID() const { return m_NormalTextureID; }
    inline StringID GetRoughnessTextureID() const { return m_RoughnessTextureID; }
    inline StringID GetMetalnessTextureID() const { return m_MetalnessTextureID; }
    inline StringID GetEmissiveTextureID() const { return m_EmissiveTextureID; }

    inline BlendMode GetBlendMode() const { return m_BlendMode; }
    inline RaytracingVisibility GetRaytracingVisibility() const { return m_RaytracingVisibility; }

    inline void SetTransientMaterialIdx(uint32_t id) { m_TransientMaterialIdx = id; }
    inline void SetBaseColor(const ethVector3& color) { m_BaseColor = color; }
    inline void SetEmissiveColor(const ethVector3& color) { m_EmissiveColor = color; }
    inline void SetRoughness(float roughness) { m_Roughness = roughness; }
    inline void SetMetalness(float metalness) { m_Metalness = metalness; }
    inline void SetOpacity(float opacity) { m_Opacity = opacity; }
    inline void SetBaseColorTextureID(const StringID& id) { m_BaseColorTextureID = id; }
    inline void SetNormalTextureID(const StringID& id) { m_NormalTextureID = id; }
    inline void SetRoughnessTextureID(const StringID& id) { m_RoughnessTextureID = id; }
    inline void SetMetalnessTextureID(const StringID& id) { m_MetalnessTextureID = id; }
    inline void SetEmissiveTextureID(const StringID& id) { m_EmissiveTextureID = id; }

    inline void SetBlendMode(BlendMode blendMode) { m_BlendMode = blendMode; }
    inline void SetRaytracingVisibility(RaytracingVisibility visibility) { m_RaytracingVisibility = visibility; }

public:
    bool HasTranslucency() const;

private:
    ethVector3 m_BaseColor;
    ethVector3 m_EmissiveColor;
    float m_Roughness;
    float m_Metalness;
    float m_Opacity;

    StringID m_BaseColorTextureID;
    StringID m_NormalTextureID;
    StringID m_RoughnessTextureID;
    StringID m_MetalnessTextureID;
    StringID m_EmissiveTextureID;

private:
    BlendMode m_BlendMode;
    RaytracingVisibility m_RaytracingVisibility;

private:
    // Transient Data
    uint32_t m_TransientMaterialIdx;
};
} // namespace Ether::Graphics
