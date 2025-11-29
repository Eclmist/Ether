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

#include "hlsltranslation.h"

ETH_BEGIN_SHADER_NAMESPACE

struct Metadata
{
    uint32_t m_EntityID;
    bool m_IsValid;
};

inline uint32_t PackMetadata(Metadata metadata)
{
    uint32_t packed;
    packed = metadata.m_EntityID & 0x7FFFFFFF;
    packed |= (uint32_t(metadata.m_IsValid) << 31);
    return packed;
}

inline Metadata UnpackMetadata(uint32_t packed)
{
    Metadata metadata;
    metadata.m_EntityID = packed & 0x7FFFFFFF;
    metadata.m_IsValid = (packed >> 31) & 0x1;
    return metadata;
}

ETH_END_SHADER_NAMESPACE
