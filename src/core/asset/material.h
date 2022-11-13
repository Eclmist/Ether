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

ETH_NAMESPACE_BEGIN

class ETH_ENGINE_DLL Material : public Asset
{
public:
    Material() = default;
    ~Material() = default;

    void Serialize(OStream& ostream) override;
    void Deserialize(IStream& istream) override;

public:
    inline ethVector4 GetBaseColor() const { return m_BaseColor; }
    inline ethVector4 GetSpecularColor() const { return m_SpecularColor; }
    inline float GetRoughness() const { return m_Roughness; }
    inline float GetMetalness() const { return m_Metalness; }

    inline void SetBaseColor(ethVector4 color) { m_BaseColor = color; }
    inline void SetSpecularColor(ethVector4 color) { m_SpecularColor = color; }
    inline void SetRoughness(float roughness) { m_Roughness = roughness; }
    inline void SetMetalness(float metalness) { m_Metalness = metalness; }

public:
    CompiledTexture* GetTexture(const std::string& key) const;
    ethVector4 GetColor(const std::string& key) const;

    void SetTexture(const std::string& key, std::shared_ptr<CompiledTexture> texture);
    void SetColor(const std::string& key, ethVector4 color);

private:
    friend class RenderingSystem;
    friend class VisualComponent;

    std::unordered_map<std::string, std::shared_ptr<CompiledTexture>> m_Textures;
    std::unordered_map<std::string, ethVector4> m_Colors;

    float m_Roughness = 0.5;
    float m_Metalness = 1.0;
    ethVector4 m_BaseColor = { 1, 1, 1, 1 };
    ethVector4 m_SpecularColor = { 1, 1, 1, 1 };

public:
    // TODO: Fix this hack
    std::string m_AlbedoTexturePath;
    std::string m_SpecularTexturePath;
    std::string m_RoughnessTexturePath;
    std::string m_MetalnessTexturePath;
};

ETH_NAMESPACE_END
