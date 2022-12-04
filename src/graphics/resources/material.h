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
        inline float GetRoughness() const { return m_Roughness; }
        inline float GetMetalness() const { return m_Metalness; }

        inline void SetBaseColor(ethVector4 color) { m_BaseColor = color; }
        inline void SetSpecularColor(ethVector4 color) { m_SpecularColor = color; }
        inline void SetRoughness(float roughness) { m_Roughness = roughness; }
        inline void SetMetalness(float metalness) { m_Metalness = metalness; }

    private:
        ethVector4 m_BaseColor = { 1.0f, 1.0f, 1.0f, 1.0f };
        ethVector4 m_SpecularColor = { 1.0f, 1.0f, 1.0f, 1.0f };
        float m_Roughness = 0.5f;
        float m_Metalness = 1.0f;
    };
}

