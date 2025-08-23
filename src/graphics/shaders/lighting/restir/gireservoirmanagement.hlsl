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
    float3 m_WeightSum;
    float3 m_TargetPdf;
    float M;


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
        reservoir.m_Sample.m_Position = packedReservoir.m_Position;
        reservoir.m_Sample.m_Normal = DecodeNormals(packedReservoir.m_PackedNormals);
        reservoir.m_Sample.m_Radiance = packedReservoir.m_Radiance;
        reservoir.m_WeightSum = packedReservoir.m_WeightSum;
        reservoir.m_TargetPdf = packedReservoir.m_TargetPdf;
        reservoir.M = packedReservoir.M;

        if (any(isinf(reservoir.m_WeightSum)) || any(isnan(reservoir.m_WeightSum)))
            return Empty();

        return reservoir;
    }

    static GIPackedReservoir Pack(GIReservoir reservoir)
    {
        GIPackedReservoir packedReservoir;
        packedReservoir.m_Position = reservoir.m_Sample.m_Position;
        packedReservoir.m_PackedNormals = EncodeNormals(reservoir.m_Sample.m_Normal);
        packedReservoir.m_Radiance = reservoir.m_Sample.m_Radiance;
        packedReservoir.m_WeightSum = reservoir.m_WeightSum;
        packedReservoir.m_TargetPdf = reservoir.m_TargetPdf;
        packedReservoir.M = reservoir.M;

        return packedReservoir;
    }

    bool IsValid()
    {
        return M > 0 && !any(isinf(m_WeightSum)) && !any(isnan(m_WeightSum));
    }

    // Streaming RIS using weighted reservoir sampling
    bool Resample(GIReservoirSample newSample, float2 random, float3 newTargetPdf, float3 risWeight)
    {
        M += 1;
        m_WeightSum += risWeight;

        const bool newSampleSelected = random.x * GetLuminanceFromRGB(m_WeightSum) <= GetLuminanceFromRGB(risWeight);

        if (newSampleSelected)
        {
            m_Sample = newSample;
            m_TargetPdf = newTargetPdf;
        }

        return newSampleSelected;
    }

    bool Combine(GIReservoir newReservoir, float2 random, float3 newTargetPdf)
    {
        if (!newReservoir.IsValid())
            return false;

        float3 risWeight = newTargetPdf * newReservoir.m_WeightSum * newReservoir.M;

        M += newReservoir.M;
        m_WeightSum += risWeight;

        const bool newSampleSelected = random.x * GetLuminanceFromRGB(m_WeightSum) <= GetLuminanceFromRGB(risWeight);

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
        const float3 denom = m_TargetPdf * M;
        m_WeightSum = (denom <= 0.0f) ? 0.0f : m_WeightSum / denom;
    }
};
