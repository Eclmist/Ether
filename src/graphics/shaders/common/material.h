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

#include "hlsltranslation.h"

ETH_BEGIN_SHADER_NAMESPACE

struct Material
{
    ethVector4 m_BaseColor;
    ethVector4 m_SpecularColor;
    ethVector4 m_EmissiveColor;

    uint32_t m_AlbedoTextureIndex;
    uint32_t m_NormalTextureIndex;
    uint32_t m_RoughnessTextureIndex;
    uint32_t m_MetalnessTextureIndex;
    uint32_t m_EmissiveTextureIndex;
};

ETH_END_SHADER_NAMESPACE
