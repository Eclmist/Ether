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
    float m_Hit;
    float m_IsShadowRay;
    uint32_t m_Depth;
    ethVector3 m_Radiance;
    ethVector3 m_HitPosition;
    ethVector3 m_HitNormal;
};

struct ReservoirSample
{
    ethVector3 m_SamplePosition;
    ethVector3 m_SampleNormal;
    ethVector3 m_Radiance;

#ifdef __HLSL__
    void Reset()
    {
        m_SamplePosition = 0;
        m_SampleNormal = 0;
        m_Radiance = 0;
    }
#endif
};

struct Reservoir
{
    ReservoirSample m_Sample;
    float m_TargetFunction;
    float m_NumSamples;
    float m_WeightSum;              

#ifdef __HLSL__
    void Reset()
    {
        m_WeightSum = 0;
        m_TargetFunction = 0;
        m_NumSamples = 0;
        m_Sample.Reset();
    }

    void InternalResample(Reservoir r, float newTargetFunction, float risWeight, float rand)
    {
        m_NumSamples += r.m_NumSamples;
        m_WeightSum += risWeight;

        if (rand * m_WeightSum < risWeight)
        {
            m_Sample = r.m_Sample;
            m_TargetFunction = newTargetFunction;
        }
    }

    void Combine(Reservoir r, float newTargetFunction, float rand)
    {
        const float ucw = r.m_WeightSum;
        const float misScalingFactor = r.m_NumSamples;
        const float risWeight = newTargetFunction * ucw * misScalingFactor;
        InternalResample(r, newTargetFunction, risWeight, rand);
    }

    void FinalizeReservoir()
    {
        // M is multiplied in the denominator to normalize MIS weights to 1 at the end
        const float denom = m_TargetFunction * m_NumSamples;

        // converts w(x) -> W(x) by dividing by p^
        m_WeightSum = denom <= 0.0f ? 0.0f : m_WeightSum / denom;
    }
#endif
};

ETH_END_SHADER_NAMESPACE
