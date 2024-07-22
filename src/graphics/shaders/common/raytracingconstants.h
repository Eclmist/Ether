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
    ethVector3 m_Wo;
    ethVector3 m_Wi;
    ethVector3 m_Albedo;
    float m_Roughness;
    float m_Metalness;

    ethVector3 m_VisiblePosition;
    ethVector3 m_VisibleNormal;
    ethVector3 m_SamplePosition;
    ethVector3 m_SampleNormal;
    ethVector3 m_Radiance;

    uint32_t m_FrameNumber;
};

struct Reservoir
{
    ReservoirSample m_Sample;
    float m_w;
    float m_W;
    uint32_t m_M;

#ifdef __HLSL__
    void Reset()
    {
        m_w = 0;
        m_W = 0;
        m_M = 0;
    }

    void Update(ReservoirSample s, float w, float rand)
    {
        m_M++;
        m_w += w;

        if (rand < w / m_w)
            m_Sample = s;
    }

    void Merge(Reservoir r, float targetPdf, float rand, uint32_t maxSamples)
    {
        float M0 = m_M;
        uint32_t incomingM = min(maxSamples, r.m_M);
        Update(r.m_Sample, targetPdf * r.m_W * incomingM, rand);
        m_M = M0 + incomingM;
    }
#endif
};

ETH_END_SHADER_NAMESPACE
