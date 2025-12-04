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

#ifndef __RESTIR_GI_TEMPORAL_RESAMPLING_CS_HLSL__
#define __RESTIR_GI_TEMPORAL_RESAMPLING_CS_HLSL__

#include "lighting/restir/gireservoirresampling.hlsl"
#include "lighting/restir/boilingfilter.hlsl"
#include "utils/random.hlsl"

bool IsValidReprojection(GIReservoirSample sample, GIReservoirSample prevSample)
{
    if (sample.m_MaterialID != prevSample.m_MaterialID)
        return false;

    /* This causes too much flickering at geometry edges
    if (dot(sample.m_VisibleNormal, prevSample.m_VisibleNormal) < 0.5f)
        return false;
    */

    if ((abs(sample.m_VisibleDepth - prevSample.m_VisibleDepth) / prevSample.m_VisibleDepth) > 0.2f)
        return false;

    return true;
}

[numthreads(THREADGROUP_SIZE, THREADGROUP_SIZE, 1)]
void CS_Main(
    uint3 threadID : SV_DispatchThreadID,
    uint3 groupThreadID : SV_GroupThreadID)
{
    const uint2 sampleCoords = threadID.xy;
    const uint2 screenSize = GlobalConstants.m_ScreenResolution.xy;
    const uint2 screenCoords = GetScreenCoordsFromSampleCoords(sampleCoords);
    const uint sampleIdx = GetSampleIndexFromScreenCoords(screenCoords, screenSize);

    if (any(screenCoords >= GlobalConstants.m_ScreenResolution.xy))
        return;
    
    const ShadingSurface surface = GetShadingSurfaceFromGBuffers(screenCoords, GBufferTextureA, GBufferTextureB, GBufferTextureC, SceneDepth);
    const float2 pixelVelocity = (surface.m_Velocity * screenSize);
    const float2 screenCoordsPrev = screenCoords - pixelVelocity + 0.5f + (Random2D(screenCoords, GlobalConstants.m_FrameNumber) - 0.5f);

    const uint prevSampleIdx = GetSampleIndexFromScreenCoords(screenCoordsPrev, screenSize);

    GIReservoir initialReservoir = GIReservoir::Unpack(InputReservoir[sampleIdx]);
    GIReservoir historyReservoir = GIReservoir::Unpack(HistoryReservoir[prevSampleIdx]);

    const float specularDependence = lerp(0.0f, lerp(1.0f, 0.0f, pow(surface.m_Roughness, 0.1f)), pow(surface.m_Metalness, 2.0f));

    if (Random(screenCoords * GlobalConstants.m_FrameNumber + 110).x > specularDependence)
    {
        if (all(screenCoordsPrev >= 0) && all(screenCoordsPrev < GlobalConstants.m_ScreenResolution.xy))
        {
            const ShadingSurface prevSurface = GetShadingSurfaceFromGBuffers(screenCoordsPrev, GBufferTextureA, GBufferTextureB, GBufferTextureC, SceneDepth);

            if (IsValidReprojection(initialReservoir.m_Sample, historyReservoir.m_Sample))
            {
                if (historyReservoir.IsValid())
                {
                    // recomputing target function here may causes a lot of inf fireflies 
                    // But not computing it can cause a color bleed from spatial feedback.
                    // For RTCamp11 Scene, recomputing looks better
                    // For Sponza, not recomputing looks better
                    const float3 targetFunction = ComputeTargetFunction(surface, historyReservoir.m_Sample);

                    historyReservoir.FinalizeResampling();
                    historyReservoir.M = min(historyReservoir.M, MAX_TEMPORAL_HISTORY);
                    initialReservoir.Combine(historyReservoir, Random(screenCoords * GlobalConstants.m_FrameNumber + 100), targetFunction);
                }
            }
        }
    }

    if (!BoilingFilter(groupThreadID.xy, 0.3f, GetLuminanceFromRGB(initialReservoir.m_WeightSum)))
        initialReservoir = GIReservoir::Empty();

    RWOutputReservoir[sampleIdx] = GIReservoir::Pack(initialReservoir);
}

#endif // __RESTIR_GI_TEMPORAL_RESAMPLING_CS_HLSL__
