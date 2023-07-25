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

#define BLOOM_KERNEL_GROUP_SIZE_X 8
#define BLOOM_KERNEL_GROUP_SIZE_Y 8
#define BLOOM_PASSINDEX_DOWNSAMPLE 0
#define BLOOM_PASSINDEX_UPSAMPLE 1
#define BLOOM_PASSINDEX_COMPOSITE 2

ETH_BEGIN_SHADER_NAMESPACE

struct BloomParams
{
    uint32_t m_PassIndex;
    ethVector2 m_Resolution;
    float m_Intensity;
    float m_Scatter;
    float m_Anamorphic;
};

ETH_END_SHADER_NAMESPACE
