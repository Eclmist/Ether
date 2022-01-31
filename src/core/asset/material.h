/*
    This file is part of Ether, an open-source DirectX 12 renderer.

    Copyright (c) 2020-2022 Samuel Huang - All rights reserved.

    Ether is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

ETH_NAMESPACE_BEGIN

class Material : public Asset
{
public:
    Material();
    ~Material() = default;

    void Serialize(OStream& ostream) override;
    void Deserialize(IStream& istream) override;

public:
    CompiledTexture* GetTexture(const std::string& key) const;
    ethVector4 GetColor(const std::string& key) const;

    void SetTexture(const std::string& key, std::shared_ptr<CompiledTexture> texture);
    void SetColor(const std::string& key, ethVector4 color);

private:
    std::unordered_map<std::string, std::shared_ptr<CompiledTexture>> m_Textures;
    std::unordered_map<std::string, ethVector4> m_Colors;
};

ETH_NAMESPACE_END
