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

#ifndef __RESTIR_GI_SPATIAL_RESAMPLING_CS_HLSL__
#define __RESTIR_GI_SPATIAL_RESAMPLING_CS_HLSL__

#include "lighting/restir/gireservoirresampling.hlsl"
#include "lighting/restir/boilingfilter.hlsl"

bool AreSurfacesSimilar(ShadingSurface thisSurface, ShadingSurface otherSurface)
{
    if (thisSurface.m_MaterialID != otherSurface.m_MaterialID)
        return false;

    if (dot(thisSurface.m_Normal, otherSurface.m_Normal) < 0.95f)
        return false;

    const float depthA = distance(thisSurface.m_Position, GlobalConstants.m_CameraPosition.xyz);
    const float depthB = distance(otherSurface.m_Position, GlobalConstants.m_CameraPosition.xyz);

    if (abs(depthA - depthB) / depthA > 0.05f)
        return false;

    return true;
}

void CalculatePartialJacobian(const float3 receiverPos, const float3 samplePos, const float3 sampleNormal,
	out float distanceToSurfaceSqr, out float cosineEmissionAngle)
{
	const float3 Vec = receiverPos - samplePos;

	distanceToSurfaceSqr = dot(Vec, Vec);
	cosineEmissionAngle = saturate(dot(sampleNormal, Vec * rsqrt(distanceToSurfaceSqr)));
}

float CalculateJacobian(float3 receiverPos, float3 neighbourReceiverPos, const GIReservoir neighbourReservoir)
{
	float originalDistanceSqr, originalCosineSqr;
	float newDistanceSqr, newCosineSqr;
	CalculatePartialJacobian(receiverPos, neighbourReservoir.m_Sample.m_SamplePosition, neighbourReservoir.m_Sample.m_SampleNormal, newDistanceSqr, newCosineSqr);
	CalculatePartialJacobian(neighbourReceiverPos, neighbourReservoir.m_Sample.m_SamplePosition, neighbourReservoir.m_Sample.m_SampleNormal, originalDistanceSqr, originalCosineSqr);

	float jacobian = (newCosineSqr * originalDistanceSqr) / (originalCosineSqr * newDistanceSqr);

	if (isinf(jacobian) || isnan(jacobian))
		jacobian = 1;

	return saturate(jacobian);
}

[numthreads(THREADGROUP_SIZE, THREADGROUP_SIZE, 1)]
void CS_Main(
    uint3 threadID : SV_DispatchThreadID,
    uint3 groupThreadID : SV_GroupThreadID)
{
    const uint2 sampleCoords = threadID.xy;
    const uint2 screenCoords = GetScreenCoordsFromSampleCoords(sampleCoords);
    const uint2 screenSize = GlobalConstants.m_ScreenResolution.xy;
    const uint sampleIdx = GetSampleIndexFromScreenCoords(screenCoords, screenSize);

    if (any(screenCoords >= GlobalConstants.m_ScreenResolution.xy))
        return;

    const ShadingSurface surface = GetShadingSurfaceFromGBuffers(screenCoords, GBufferTextureA, GBufferTextureB, GBufferTextureC, SceneDepth);

    GIReservoir initialReservoir = GIReservoir::Unpack(InputReservoir[sampleIdx]);
    const uint numSamples = initialReservoir.M < 5 ? NUM_SPATIAL_SAMPLES * 2.0f : NUM_SPATIAL_SAMPLES;
    const float lowHistorySampleMultiplier = 1.5f;
    const uint historyAwareSpatialSampleCount = NUM_SPATIAL_SAMPLES * max(1, lowHistorySampleMultiplier - (initialReservoir.M / (MAX_TEMPORAL_HISTORY / lowHistorySampleMultiplier)));

    for (int i = 0; i < numSamples; ++i)
    {
        const float goldenAngle = 2.3999632f;
        const float angle = (i + Random(screenCoords * GlobalConstants.m_FrameNumber + 200) * 3.1415) * goldenAngle;
        const float materialFactor = max(surface.m_Roughness, 1.0f - surface.m_Metalness);
        const float radius = pow(float(i + 1.0f), 0.666f) * SPATIAL_KERNEL_RADIUS / (float) numSamples * surface.m_Roughness;
        const float2 offset = float2(cos(angle), sin(angle)) * radius;
        const int2 neighbourScreenCoords = screenCoords + offset;
        const uint neighbourSampleIdx = GetSampleIndexFromScreenCoords(neighbourScreenCoords, screenSize);

        const ShadingSurface neighbourSurface = GetShadingSurfaceFromGBuffers(neighbourScreenCoords, GBufferTextureA, GBufferTextureB, GBufferTextureC, SceneDepth);

        if (any(neighbourScreenCoords < 0) || any(neighbourScreenCoords >= screenSize))
            continue;

        if (!AreSurfacesSimilar(surface, neighbourSurface))
            continue;

        GIReservoir neighbourReservoir = GIReservoir::Unpack(InputReservoir[neighbourSampleIdx]);

        if (!neighbourReservoir.IsValid())
            continue;

        const float jacobian = CalculateJacobian(surface.m_Position, neighbourSurface.m_Position, neighbourReservoir);
        const float3 targetFunction = ComputeTargetFunction(surface, neighbourReservoir.m_Sample);

        // surface detail is somehow lost if this is added
        // neighbourReservoir.m_TargetPdf = targetFunction;

        neighbourReservoir.FinalizeResampling();
        neighbourReservoir.M = min(neighbourReservoir.M,  100);
        initialReservoir.Combine(neighbourReservoir, Random(screenCoords * GlobalConstants.m_FrameNumber + 300), targetFunction);
    }

    RWOutputReservoir[sampleIdx] = GIReservoir::Pack(initialReservoir);
}

#endif // __RESTIR_GI_SPATIAL_RESAMPLING_CS_HLSL__

