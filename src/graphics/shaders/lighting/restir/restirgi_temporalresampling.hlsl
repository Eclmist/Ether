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

bool IsValidReprojection(uint2 screenCoords, uint2 prevScreenCoords)
{
    const ShadingSurface surface = GetShadingSurfaceFromGBuffers(screenCoords, g_GBufferA, g_GBufferB, g_GBufferC, g_GBufferD);
    const ShadingSurface prevSurface = GetShadingSurfaceFromGBuffers(prevScreenCoords, g_GBufferA, g_GBufferB, g_GBufferC, g_GBufferD);

return true;

    if (dot(surface.m_Normal, prevSurface.m_Normal) < 0.8f)
        return false;


    return true;
}

[numthreads(32, 32, 1)]
void CS_Main(uint3 threadID : SV_DispatchThreadID)
{
    const uint2 screenCoords = threadID.xy;
    const uint2 screenDims = g_GlobalConstants.m_ScreenResolution.xy;
    const uint sampleIdx = screenCoords.y * screenDims.x + screenCoords.x;
    const ShadingSurface surface = GetShadingSurfaceFromGBuffers(screenCoords, g_GBufferA, g_GBufferB, g_GBufferC, g_GBufferD);

    const int2 prevScreenCoords = clamp(((float2)screenCoords + 0.5) - (surface.m_Velocity * screenDims), 0, screenDims);
    const uint prevSampleIdx = prevScreenCoords.y * screenDims.x + prevScreenCoords.x;

    GIReservoir initialReservoir = GIReservoir::Unpack(g_InputReservoir[sampleIdx]);

    if (IsValidReprojection(screenCoords, prevScreenCoords))
    {
        GIReservoir historyReservoir = GIReservoir::Unpack(g_HistoryReservoir[prevSampleIdx]);

        if (historyReservoir.IsValid())
        {
            const float targetFunction = EvaluateTargetFunction(surface, historyReservoir.m_Sample);

            historyReservoir.FinalizeResampling();
            historyReservoir.M = min(historyReservoir.M, 3);
            initialReservoir.Combine(historyReservoir, Random(sampleIdx * g_GlobalConstants.m_FrameNumber), targetFunction);
        }
    }

    g_RWOutputReservoir[sampleIdx] = GIReservoir::Pack(initialReservoir);
    //g_RWOutputReservoir[sampleIdx] = GIReservoir::Pack(GIReservoir::Empty());
}
