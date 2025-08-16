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

// TODO: Make into cvar
#define NUM_SPATIAL_SAMPLES 8
#define SPATIAL_KERNEL_RADIUS 32

bool AreSurfacesSimilar(uint2 screenCoords, uint2 prevScreenCoords)
{
    const ShadingSurface surface = GetShadingSurfaceFromGBuffers(screenCoords, g_GBufferA, g_GBufferB, g_GBufferC, g_GBufferD);
    const ShadingSurface prevSurface = GetShadingSurfaceFromGBuffers(prevScreenCoords, g_GBufferA, g_GBufferB, g_GBufferC, g_GBufferD);

    if (dot(surface.m_Normal, prevSurface.m_Normal) < 0.8f)
        return false;

    if (distance(surface.m_Position, surface.m_Position) > 0.5f)
        return false;

    if (abs(surface.m_Metalness - prevSurface.m_Metalness) > 0.1f)
        return false;

    if (abs(surface.m_Roughness - prevSurface.m_Roughness) > 0.1f)
        return false;

    return true;
}

void CalculatePartialJacobian(const float3 RecieverPos, const float3 SamplePos, const float3 SampleNormal,
	out float DistanceToSurfaceSqr, out float CosineEmissionAngle)
{
	const float3 Vec = RecieverPos - SamplePos;

	DistanceToSurfaceSqr = dot(Vec, Vec);
	CosineEmissionAngle = saturate(dot(SampleNormal, Vec * rsqrt(DistanceToSurfaceSqr)));
}

float CalculateJacobian(float3 RecieverPos, float3 NeighborReceiverPos, const GIReservoir NeighborReservoir)
{
	float OriginalDistanceSqr, OriginalCosine;
	float NewDistanceSqr, NewCosine;
	CalculatePartialJacobian(RecieverPos, NeighborReservoir.m_Sample.m_Position, NeighborReservoir.m_Sample.m_Normal, NewDistanceSqr, NewCosine);
	CalculatePartialJacobian(NeighborReceiverPos, NeighborReservoir.m_Sample.m_Position, NeighborReservoir.m_Sample.m_Normal, OriginalDistanceSqr, OriginalCosine);

	float Jacobian = (NewCosine * OriginalDistanceSqr) / (OriginalCosine * NewDistanceSqr);

	if (isinf(Jacobian) || isnan(Jacobian))
		Jacobian = 1;

	return saturate(Jacobian);
}

[numthreads(THREADGROUP_SIZE, THREADGROUP_SIZE, 1)]
void CS_Main(uint3 threadID : SV_DispatchThreadID)
{
    const uint2 sampleCoords = threadID.xy;
    const uint2 screenCoords = GetScreenCoordsFromSampleCoords(sampleCoords);
    const uint2 screenSize = g_GlobalConstants.m_ScreenResolution.xy;
    const uint sampleIdx = GetSampleIndexFromScreenCoords(screenCoords, screenSize);
    const ShadingSurface surface = GetShadingSurfaceFromGBuffers(screenCoords, g_GBufferA, g_GBufferB, g_GBufferC, g_GBufferD);

    GIReservoir initialReservoir = GIReservoir::Unpack(g_InputReservoir[sampleIdx]);

    for (int i = 0; i < NUM_SPATIAL_SAMPLES; ++i)
    {
        const float goldenAngle = 2.3999632f;
        const float angle = (i + Random(screenCoords * g_GlobalConstants.m_FrameNumber + 200) * 3.1415) * goldenAngle;
        const float radius = pow(float(i + 1.0f), 0.666f) * SPATIAL_KERNEL_RADIUS / (float)NUM_SPATIAL_SAMPLES;
        const float2 offset = float2(cos(angle), sin(angle)) * radius;
        const int2 neighbourScreenCoords = screenCoords + offset;
        const uint neighbourSampleIdx = GetSampleIndexFromScreenCoords(neighbourScreenCoords, screenSize);

        const ShadingSurface neighbourSurface = GetShadingSurfaceFromGBuffers(neighbourScreenCoords, g_GBufferA, g_GBufferB, g_GBufferC, g_GBufferD);

        if (any(neighbourScreenCoords < 0) || any(neighbourScreenCoords >= screenSize))
            continue;

        if (!AreSurfacesSimilar(screenCoords, neighbourScreenCoords))
            continue;

        GIReservoir neighbourReservoir = GIReservoir::Unpack(g_InputReservoir[neighbourSampleIdx]);
        const float jacobian = CalculateJacobian(surface.m_Position, neighbourSurface.m_Position, neighbourReservoir);
        neighbourReservoir.m_WeightSum *= jacobian;

        if (!neighbourReservoir.IsValid())
            continue;

        const float targetFunction = EvaluateTargetFunction(surface, neighbourReservoir.m_Sample);
        neighbourReservoir.FinalizeResampling();
        neighbourReservoir.M = min(neighbourReservoir.M, 200);
        initialReservoir.Combine(neighbourReservoir, Random(screenCoords * g_GlobalConstants.m_FrameNumber + 200), targetFunction);
    }

    g_RWOutputReservoir[sampleIdx] = GIReservoir::Pack(initialReservoir);
}
