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

struct ReservoirSample
{
    ethVector3 m_VisiblePosition;
    ethVector3 m_VisibleNormal;
    ethVector3 m_SamplePosition;
    ethVector3 m_SampleNormal;
    ethVector3 m_Radiance;
    ethVector3 m_Random;
};

struct Reservoir
{
    ReservoirSample m_Sample; // RIS - z
    float m_TotalW; // RIS - w
    float m_W; // ReSTIR GI - W
    uint32_t m_M; // RIS - M

#ifdef __HLSL__
    void Reset()
    { 
        m_TotalW = 0;
        m_W = 0;
        m_M = 0;
    }

    void Update(ReservoirSample s, float newWeight, float rand, uint32_t maxM)
    {
        if (m_M >= maxM)
        {
            m_M = maxM;
            m_TotalW -= m_TotalW / m_M;
        }

        m_M++;
        m_TotalW += newWeight;

        if (rand < newWeight / m_TotalW)
            m_Sample = s;
    }

    void Merge(Reservoir r, float targetPdf, float rand, uint32_t maxM)
    {
        float M0 = m_M;
        Update(r.m_Sample, targetPdf * r.m_W * r.m_M, rand, maxM);
        m_M = M0 + r.m_M;
    }
#endif
};

ETH_END_SHADER_NAMESPACE
