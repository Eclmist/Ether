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

bool AreSurfacesSimilar(ShadingSurface thisSurface, ShadingSurface otherSurface)
{
    if (thisSurface.m_MaterialID != otherSurface.m_MaterialID)
        return false;

    if (dot(thisSurface.m_Normal, otherSurface.m_Normal) < 0.95f)
        return false;

    const float depthA = distance(thisSurface.m_Position, g_GlobalConstants.m_CameraPosition.xyz);
    const float depthB = distance(otherSurface.m_Position, g_GlobalConstants.m_CameraPosition.xyz);

    if (abs(depthA - depthB) / depthA > 0.05f)
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
	CalculatePartialJacobian(RecieverPos, NeighborReservoir.m_Sample.m_SamplePosition, NeighborReservoir.m_Sample.m_SampleNormal, NewDistanceSqr, NewCosine);
	CalculatePartialJacobian(NeighborReceiverPos, NeighborReservoir.m_Sample.m_SamplePosition, NeighborReservoir.m_Sample.m_SampleNormal, OriginalDistanceSqr, OriginalCosine);

	float Jacobian = (NewCosine * OriginalDistanceSqr) / (OriginalCosine * NewDistanceSqr);

	if (isinf(Jacobian) || isnan(Jacobian))
		Jacobian = 1;

	return saturate(Jacobian);
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

    if (any(screenCoords >= g_GlobalConstants.m_ScreenResolution.xy))
        return;

    const ShadingSurface surface = GetShadingSurfaceFromGBuffers(screenCoords, g_GBufferA, g_GBufferB, g_GBufferC, g_SceneDepth);

    GIReservoir initialReservoir = GIReservoir::Unpack(g_InputReservoir[sampleIdx]);
    const uint numSamples = initialReservoir.M < 5 ? NUM_SPATIAL_SAMPLES * 2.0f : NUM_SPATIAL_SAMPLES;
    const float lowHistorySampleMultiplier = 1.5f;
    const uint historyAwareSpatialSampleCount = NUM_SPATIAL_SAMPLES * max(1, lowHistorySampleMultiplier - (initialReservoir.M / (MAX_TEMPORAL_HISTORY / lowHistorySampleMultiplier)));

    for (int i = 0; i < numSamples; ++i)
    {
        const float goldenAngle = 2.3999632f;
        const float angle = (i + Random(screenCoords * g_GlobalConstants.m_FrameNumber + 200) * 3.1415) * goldenAngle;
        const float materialFactor = max(surface.m_Roughness, 1.0f - surface.m_Metalness);
        const float radius = pow(float(i + 1.0f), 0.666f) * SPATIAL_KERNEL_RADIUS / (float) numSamples * surface.m_Roughness;
        const float2 offset = float2(cos(angle), sin(angle)) * radius;
        const int2 neighbourScreenCoords = screenCoords + offset;
        const uint neighbourSampleIdx = GetSampleIndexFromScreenCoords(neighbourScreenCoords, screenSize);

        const ShadingSurface neighbourSurface = GetShadingSurfaceFromGBuffers(neighbourScreenCoords, g_GBufferA, g_GBufferB, g_GBufferC, g_SceneDepth);

        if (any(neighbourScreenCoords < 0) || any(neighbourScreenCoords >= screenSize))
            continue;

        if (!AreSurfacesSimilar(surface, neighbourSurface))
            continue;

        GIReservoir neighbourReservoir = GIReservoir::Unpack(g_InputReservoir[neighbourSampleIdx]);

        if (!neighbourReservoir.IsValid())
            continue;

        const float jacobian = CalculateJacobian(surface.m_Position, neighbourSurface.m_Position, neighbourReservoir);
        const float3 targetFunction = ComputeTargetFunction(surface, neighbourReservoir.m_Sample) * jacobian;

        // surface detail is somehow lost if this is added
        // neighbourReservoir.m_TargetPdf = targetFunction;

        neighbourReservoir.FinalizeResampling();
        neighbourReservoir.M = min(neighbourReservoir.M,  100);
        initialReservoir.Combine(neighbourReservoir, Random(screenCoords * g_GlobalConstants.m_FrameNumber + 300), targetFunction);
    }

    g_RWOutputReservoir[sampleIdx] = GIReservoir::Pack(initialReservoir);
}
