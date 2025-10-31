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

bool IsValidReprojection(ShadingSurface surface, ShadingSurface prevSurface)
{
    if (dot(surface.m_Normal, prevSurface.m_Normal) < 0.8f)
        return false;

    const float depthA = distance(surface.m_Position, g_GlobalConstants.m_CameraPosition.xyz);
    const float depthB = distance(prevSurface.m_Position, g_GlobalConstants.m_CameraPosition.xyz);

    if (abs(depthA - depthB) / depthA > 0.15f)
        return false;

    return true;
}

[numthreads(THREADGROUP_SIZE, THREADGROUP_SIZE, 1)]
void CS_Main(
    uint3 threadID : SV_DispatchThreadID,
    uint3 groupThreadID : SV_GroupThreadID)
{
    const uint2 sampleCoords = threadID.xy;
    const uint2 screenSize = g_GlobalConstants.m_ScreenResolution.xy;
    const uint2 screenCoords = GetScreenCoordsFromSampleCoords(sampleCoords);
    const uint sampleIdx = GetSampleIndexFromScreenCoords(screenCoords, screenSize);

    if (any(screenCoords < 0) || any(screenCoords >= g_GlobalConstants.m_ScreenResolution.xy))
        return;
    
    const ShadingSurface surface = GetShadingSurfaceFromGBuffers(screenCoords, g_GBufferA, g_GBufferB, g_GBufferC, g_GBufferD);
    const float2 pixelJitterDelta = g_GlobalConstants.m_CameraJitterPrev - g_GlobalConstants.m_CameraJitter;
    const float2 pixelVelocity = surface.m_Velocity * screenSize;
    const float2 prevScreenCoords = floor((float2(screenCoords) + 0.5f) - pixelVelocity + pixelJitterDelta);
    const uint prevSampleIdx = GetSampleIndexFromScreenCoords(prevScreenCoords, screenSize);

    GIReservoir initialReservoir = GIReservoir::Unpack(g_InputReservoir[sampleIdx]);
    GIReservoir historyReservoir = GIReservoir::Unpack(g_HistoryReservoir[prevSampleIdx]);

    const float specularDependence = lerp(0.0f, lerp(1.0f, 0.0f, pow(surface.m_Roughness, 0.1f)), pow(surface.m_Metalness, 2.0f));

    if (Random(screenCoords, g_GlobalConstants.m_FrameNumber + 110).x > specularDependence)
    {
        if (all(prevScreenCoords >= 0) && all(prevScreenCoords < g_GlobalConstants.m_ScreenResolution.xy))
        {
            const ShadingSurface prevSurface = GetShadingSurfaceFromGBuffers(prevScreenCoords, g_GBufferA, g_GBufferB, g_GBufferC, g_GBufferD);

            if (IsValidReprojection(surface, prevSurface))
            {
                if (historyReservoir.IsValid())
                {
                    const float3 targetFunction = ComputeTargetFunction(surface, historyReservoir.m_Sample);

                    historyReservoir.FinalizeResampling();
                    historyReservoir.M = min(historyReservoir.M, MAX_TEMPORAL_HISTORY);
                    initialReservoir.Combine(historyReservoir, Random(screenCoords, g_GlobalConstants.m_FrameNumber + 100), targetFunction);
                }
            }
        }
    }

    g_RWOutputReservoir[sampleIdx] = GIReservoir::Pack(initialReservoir);
}
