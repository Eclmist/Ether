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

#include "common/raytracingconstants.h"

// reservoir.m_PackedData1.x: [ Position.x (fp32) ]
// reservoir.m_PackedData1.y: [ Position.y (fp32) ]
// reservoir.m_PackedData1.z: [ Position.z (fp32) ]
// reservoir.m_PackedData1.w: [ TargetPdf  (fp32) ]
// reservoir.m_PackedData2.x: [ Normal.x   (fp32) ]
// reservoir.m_PackedData2.y: [ Normal.y   (fp32) ]
// reservoir.m_PackedData2.z: [ Normal.z   (fp32) ]
// reservoir.m_PackedData2.w: [ WeightSum  (fp32) ]
// reservoir.m_PackedData3.x: [ Radiance.x (fp32) ]
// reservoir.m_PackedData3.y: [ Radiance.y (fp32) ]
// reservoir.m_PackedData3.z: [ Radiance.z (fp32) ]
// reservoir.m_PackedData3.w: [ M          (fp32) ]

struct GIReservoirSample
{
    float3 m_Position;
    float3 m_Normal;
    float3 m_Radiance;

    static GIReservoirSample Empty()
    {
        GIReservoirSample sample;
        sample.m_Position = 0;
        sample.m_Normal = 0;
        sample.m_Radiance = 0;
        return sample;
    }
};

struct GIReservoir
{
    GIReservoirSample m_Sample;
    float m_WeightSum;
    float m_TargetPdf;
    uint M;

    static GIReservoir Empty()
    {
        GIReservoir reservoir;
        reservoir.m_Sample = GIReservoirSample::Empty();
        reservoir.m_WeightSum = 0;
        reservoir.m_TargetPdf = 0;
        reservoir.M = 0;
        return reservoir;
    }

    static GIReservoir Unpack(GIPackedReservoir packedReservoir)
    {
        GIReservoir reservoir;
        reservoir.m_Sample.m_Position = packedReservoir.m_PackedData1.xyz;
        reservoir.m_Sample.m_Normal = packedReservoir.m_PackedData2.xyz;
        reservoir.m_Sample.m_Radiance = packedReservoir.m_PackedData3.xyz;
        reservoir.m_TargetPdf = packedReservoir.m_PackedData1.w;
        reservoir.m_WeightSum = packedReservoir.m_PackedData2.w;
        reservoir.M = packedReservoir.m_PackedData3.w;

        if (isinf(reservoir.m_WeightSum) || isnan(reservoir.m_WeightSum))
            return Empty();

        return reservoir;
    }

    static GIPackedReservoir Pack(GIReservoir reservoir)
    {
        GIPackedReservoir packedReservoir;
        packedReservoir.m_PackedData1.xyz = reservoir.m_Sample.m_Position;
        packedReservoir.m_PackedData2.xyz = reservoir.m_Sample.m_Normal;
        packedReservoir.m_PackedData3.xyz = reservoir.m_Sample.m_Radiance;
        packedReservoir.m_PackedData1.w = reservoir.m_TargetPdf;
        packedReservoir.m_PackedData2.w = reservoir.m_WeightSum;
        packedReservoir.m_PackedData3.w = reservoir.M;
        return packedReservoir;
    }

    bool IsValid()
    {
        return M > 0 && !isinf(m_WeightSum) && !isnan(m_WeightSum);
    }

    // Streaming RIS using weighted reservoir sampling
    bool Resample(GIReservoirSample newSample, float random, float newTargetPdf, float risWeight)
    {
        M += 1;
        m_WeightSum += risWeight;

        bool newSampleSelected = random * m_WeightSum <= risWeight;

        if (newSampleSelected)
        {
            m_Sample = newSample;
            m_TargetPdf = newTargetPdf;
        }

        return newSampleSelected;
    }

    bool Combine(GIReservoir newReservoir, float random, float newTargetPdf)
    {
        if (!newReservoir.IsValid())
            return false;

        float risWeight = newTargetPdf * newReservoir.m_WeightSum * newReservoir.M;

        M += newReservoir.M;
        m_WeightSum += risWeight;

        bool newSampleSelected = random * m_WeightSum <= risWeight;

        if (newSampleSelected)
        {
            m_Sample = newReservoir.m_Sample;
            m_TargetPdf = newTargetPdf;
        }

        return newSampleSelected;
    }

    // Calculates Unbiased Contribution Weight into WeightSum
    // Should be called when treating this reservoir as a single RIS sample, right before combining with other
    // reservoirs
    void FinalizeResampling()
    {
        const float denom = m_TargetPdf * M;
        m_WeightSum = (denom <= 0.0f) ? 0.0f : m_WeightSum / denom;
    }
};
