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

#ifndef __RESERVOIR_MANAGEMENT_HLSL__
#define __RESERVOIR_MANAGEMENT_HLSL__

#include "common/raytracingconstants.h"

struct GIReservoirSample
{
    uint m_MaterialID;
    float m_VisibleDepth;
    float3 m_VisibleNormal;
    float3 m_SamplePosition;
    float3 m_SampleNormal;
    float3 m_Radiance;

    static GIReservoirSample Empty()
    {
        GIReservoirSample sample;
        sample.m_MaterialID = 0;
        sample.m_VisibleDepth = 0;
        sample.m_VisibleNormal = 0;
        sample.m_SamplePosition = 0;
        sample.m_SampleNormal = 0;
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
        reservoir.m_Sample.m_VisibleNormal = DecodeNormals(packedReservoir.m_PackedNormals.xy);
        reservoir.m_Sample.m_SampleNormal = DecodeNormals(packedReservoir.m_PackedNormals.zw);
        reservoir.m_WeightSum = packedReservoir.m_WeightSum;
        reservoir.m_TargetPdf = packedReservoir.m_TargetPdf;


        reservoir.m_Sample.m_MaterialID = packedReservoir.m_PackedData0 & 0xFFFF;
        reservoir.m_Sample.m_VisibleDepth = f16tof32(packedReservoir.m_PackedData0 >> 16);

        reservoir.M = float(packedReservoir.m_PackedData1 >> 16);

        reservoir.m_Sample.m_SamplePosition.x = f16tof32(packedReservoir.m_PackedData1 & 0xFFFF);
        reservoir.m_Sample.m_SamplePosition.y = f16tof32(packedReservoir.m_PackedData2 >> 16);
        reservoir.m_Sample.m_SamplePosition.z = f16tof32(packedReservoir.m_PackedData2 & 0xFFFF);

        reservoir.m_Sample.m_Radiance.x = f16tof32(packedReservoir.m_PackedData3 >> 16);
        reservoir.m_Sample.m_Radiance.y = f16tof32(packedReservoir.m_PackedData3 & 0xFFFF);
        reservoir.m_Sample.m_Radiance.z = f16tof32(packedReservoir.m_PackedData4 >> 16);

        if (any(isinf(reservoir.m_WeightSum)) || any(isnan(reservoir.m_WeightSum)))
            return Empty();

        return reservoir;
    }

    static GIPackedReservoir Pack(GIReservoir reservoir)
    {
        GIPackedReservoir packedReservoir;
        packedReservoir.m_PackedNormals.xy = OctahedralEncode(reservoir.m_Sample.m_VisibleNormal);
        packedReservoir.m_PackedNormals.zw = OctahedralEncode(reservoir.m_Sample.m_SampleNormal);
        packedReservoir.m_WeightSum = reservoir.m_WeightSum;
        packedReservoir.m_TargetPdf = reservoir.m_TargetPdf;

        packedReservoir.m_PackedData0 = (f32tof16(reservoir.m_Sample.m_VisibleDepth) << 16);
        packedReservoir.m_PackedData0 |= min(0xFFFF, reservoir.m_Sample.m_MaterialID) & 0xFFFF;

        packedReservoir.m_PackedData1 = min(0xFFFF, (uint) reservoir.M) << 16;
        packedReservoir.m_PackedData1 |= f32tof16(reservoir.m_Sample.m_SamplePosition.x);
        packedReservoir.m_PackedData2 = f32tof16(reservoir.m_Sample.m_SamplePosition.y) << 16;
        packedReservoir.m_PackedData2 |= f32tof16(reservoir.m_Sample.m_SamplePosition.z) & 0xFFFF;

        packedReservoir.m_PackedData3 = f32tof16(reservoir.m_Sample.m_Radiance.x) << 16;
        packedReservoir.m_PackedData3 |= f32tof16(reservoir.m_Sample.m_Radiance.y) & 0xFFFF;
        packedReservoir.m_PackedData4 = f32tof16(reservoir.m_Sample.m_Radiance.z) << 16;

        return packedReservoir;
    }

    bool IsValid()
    {
        return M > 0 && !any(isinf(m_WeightSum)) && !any(isnan(m_WeightSum));
    }

    // Streaming RIS using weighted reservoir sampling
    bool Resample(GIReservoirSample newSample, float random, float3 newTargetPdf, float3 risWeight)
    {
        M += 1;
        m_WeightSum += risWeight;

        const bool newSampleSelected = random * GetLuminanceFromRGB(m_WeightSum) <= GetLuminanceFromRGB(risWeight);

        if (newSampleSelected)
        {
            m_Sample = newSample;
            m_TargetPdf = newTargetPdf;
        }

        return newSampleSelected;
    }

    bool Combine(GIReservoir newReservoir, float random, float3 newTargetPdf)
    {
        if (!newReservoir.IsValid())
            return false;

        float3 risWeight = newTargetPdf * newReservoir.m_WeightSum * newReservoir.M;

        M += newReservoir.M;
        m_WeightSum += risWeight;

        const bool newSampleSelected = random * GetLuminanceFromRGB(m_WeightSum) <= GetLuminanceFromRGB(risWeight);

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
        m_WeightSum = (any(denom <= 0.0f)) ? 0.0f : m_WeightSum / denom;
    }
};

#endif // __RESERVOIR_MANAGEMENT_HLSL__
