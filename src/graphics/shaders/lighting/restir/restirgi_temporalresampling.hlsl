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

#include "lighting/restir/gireservoirresampling.hlsl"
#include "lighting/restir/boilingfilter.hlsl"

#define MAX_TEMPORAL_HISTORY 30

bool IsValidReprojection(uint2 screenCoords, uint2 prevScreenCoords)
{
    const ShadingSurface surface = GetShadingSurfaceFromGBuffers(screenCoords, g_GBufferA, g_GBufferB, g_GBufferC, g_GBufferD);
    const ShadingSurface prevSurface = GetShadingSurfaceFromGBuffers(prevScreenCoords, g_GBufferA, g_GBufferB, g_GBufferC, g_GBufferD);

    if (any(prevScreenCoords < 0) || any(prevScreenCoords >= g_GlobalConstants.m_ScreenResolution.xy))
        return false;

    if (dot(surface.m_Normal, prevSurface.m_Normal) < 0.8f)
        return false;

    if (distance(surface.m_Position, surface.m_Position) > 0.5f)
        return false;

    return true;
}

[numthreads(THREADGROUP_SIZE, THREADGROUP_SIZE, 1)]
void CS_Main(
    uint3 threadID : SV_DispatchThreadID,
    uint3 groupThreadID : SV_GroupThreadID)
{
    const uint2 sampleCoords = threadID.xy;
    const uint2 screenCoords = GetScreenCoordsFromSampleCoords(sampleCoords);
    const uint2 screenSize = g_GlobalConstants.m_ScreenResolution.xy;
    const uint sampleIdx = GetSampleIndexFromScreenCoords(screenCoords, screenSize);
    const ShadingSurface surface = GetShadingSurfaceFromGBuffers(screenCoords, g_GBufferA, g_GBufferB, g_GBufferC, g_GBufferD);

    const uint2 prevScreenCoords = ((float2)screenCoords + 0.5f) - (surface.m_Velocity * screenSize);
    const uint prevSampleIdx = GetSampleIndexFromScreenCoords(prevScreenCoords, screenSize);

    if (any(screenCoords < 0) || any(screenCoords >= g_GlobalConstants.m_ScreenResolution.xy))
        return;

    GIReservoir initialReservoir = GIReservoir::Unpack(g_InputReservoir[sampleIdx]);

    if (initialReservoir.IsValid())
    {
        if (IsValidReprojection(screenCoords, prevScreenCoords))
        {
            GIReservoir historyReservoir = GIReservoir::Unpack(g_HistoryReservoir[prevSampleIdx]);

            const bool boilingFilter = BoilingFilter(groupThreadID.xy, 0.5f, historyReservoir.m_WeightSum);

            if (historyReservoir.IsValid() && boilingFilter)
            {
                const float targetFunction = EvaluateTargetFunction(surface, historyReservoir.m_Sample);
                historyReservoir.FinalizeResampling();
                historyReservoir.M = min(historyReservoir.M, MAX_TEMPORAL_HISTORY);
                initialReservoir.Combine(historyReservoir, Random(screenCoords * g_GlobalConstants.m_FrameNumber + 100), targetFunction);
            }
        }
    }

    g_RWOutputReservoir[sampleIdx] = GIReservoir::Pack(initialReservoir);
}
