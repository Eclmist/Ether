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

#define RESTIR_INITIAL_PASS     0
#define RESTIR_TEMPORAL_PASS    1
#define RESTIR_SPATIAL_PASS     2
#define RESTIR_EVALUATION_PASS  3

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

#ifdef __HLSL__
    void Reset()
    {
        m_VisiblePosition = 0;
        m_VisibleNormal = 0;
        m_SamplePosition = 0;
        m_SampleNormal = 0;
        m_Radiance = 0;
    }
#endif
};

struct Reservoir
{
    ReservoirSample m_Sample;
    float m_TotalWeight;
    float m_UnbiasedContributionWeight;
    float m_NumSamples;

#ifdef __HLSL__
    void Reset()
    {
        m_TotalWeight = 0;
        m_UnbiasedContributionWeight = 0;
        m_NumSamples = 0;
        m_Sample.Reset();
    }

    void Update(ReservoirSample s, float w, float rand)
    {
        m_NumSamples++;
        m_TotalWeight += w;

        if (rand <= w / m_TotalWeight)
            m_Sample = s;
    }

    void Merge(Reservoir r, float targetFunc, float rand, uint32_t maxHistory)
    {
        float M0 = m_NumSamples;
        float clampedM = min(maxHistory, r.m_NumSamples);
        Update(r.m_Sample, targetFunc * r.m_UnbiasedContributionWeight * clampedM, rand);
        m_NumSamples = M0 + clampedM;
    }

    //bool InternalResample(Reservoir r, float newTargetPdf, float rand, float sampleNormalization)
    //{
    //    // p^ * r.W * r.M
    //    float risWeight = newTargetPdf * sampleNormalization;
    //    m_NumSamples += r.m_NumSamples;
    //    m_TotalWeight += risWeight;

    //    const bool shouldReplaceSample = rand < risWeight / m_TotalWeight;

    //    if (shouldReplaceSample)
    //    {
    //        m_Sample = r.m_Sample;
    //        m_TargetPdf = newTargetPdf;
    //    }

    //    return shouldReplaceSample;
    //}


    //bool Combine(Reservoir r, float newTargetPdf, float rand, uint32_t maxHistory)
    //{
    //    //r.m_NumSamples = min(maxHistory, r.m_NumSamples);
    //    return InternalResample(r, newTargetPdf, rand, r.m_TotalWeight / r.m_NumSamples);
    //}
#endif
};

ETH_END_SHADER_NAMESPACE
