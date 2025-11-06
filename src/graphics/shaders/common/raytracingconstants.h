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

#define INDIRECT_MIP_LEVEL  8
#define MAX_DEPTH           2

// TODO: implement various biases (pullback, surface, etc.)
#define RAY_TMAX            128
#define RAY_TMIN            0.1

#define USE_IMPORTANCE_SAMPLING 1

struct GeometryInfo
{
    uint32_t m_VBDescriptorIndex;
    uint32_t m_IBDescriptorIndex;
    uint32_t m_MaterialIndex;
    uint32_t m_PadTo16Bytes;
};

struct RayPayload
{
    bool m_Hit;
    bool m_IsShadowRay;
    uint32_t m_Depth;
    ethVector3 m_Radiance;
    ethVector3 m_HitPosition;
    ethVector3 m_HitNormal;
};

struct GIPackedReservoir
{
    ethVector4 m_PackedNormals;
    ethVector3 m_SamplePosition;
    ethVector3 m_Radiance;
    ethVector3 m_WeightSum;
    ethVector3 m_TargetPdf;
    float M;

    uint32_t m_PackedData0;
};

ETH_END_SHADER_NAMESPACE
