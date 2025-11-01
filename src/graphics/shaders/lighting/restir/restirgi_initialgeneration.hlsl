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

// TODO: Move to common
float LinearizeDepth(float depth)
{
    float near = g_GlobalConstants.m_CameraClipNearFar.x;
    float far = g_GlobalConstants.m_CameraClipNearFar.y;
    return (far * near) / ((near - far) * depth + far);
}

[shader("raygeneration")]
void RayGeneration()
{
    const uint2 sampleCoords = DispatchRaysIndex().xy;
    const uint2 screenCoords = GetScreenCoordsFromSampleCoords(sampleCoords);
    const uint2 bufferSize = DispatchRaysDimensions().xy;
    const uint sampleIdx = GetSampleIndexFromSampleCoords(sampleCoords, bufferSize);

    if (any(screenCoords >= g_GlobalConstants.m_ScreenResolution.xy))
        return;

    const ShadingSurface surface = GetShadingSurfaceFromGBuffers(screenCoords, g_GBufferA, g_GBufferB, g_GBufferC, g_SceneDepth);
    const float depth = g_SceneDepth.Load(int3(screenCoords, 0)).r;

    const float3 viewDir = normalize(g_GlobalConstants.m_CameraPosition.xyz - surface.m_Position);
    const float3 wo = normalize(g_GlobalConstants.m_CameraPosition.xyz - surface.m_Position);

    float3 wi;
    float pdf;

#if USE_IMPORTANCE_SAMPLING
    SampleDirectionBrdf(surface, g_GlobalConstants.m_FrameNumber, viewDir, wi, pdf);
#else
    SampleDirectionUniform(surface, g_GlobalConstants.m_FrameNumber, wi, pdf);
#endif

    GIReservoir initialReservoir = GIReservoir::Empty();

    if (pdf > 0.01f)
    {
        const RayPayload payload = TraceShadingRay(surface, wi, MAX_DEPTH);
        GIReservoirSample initialSample = GIReservoirSample::Empty();
        initialSample.m_VisibleDepth = LinearizeDepth(depth);
        initialSample.m_VisibleNormal = surface.m_Normal;
        initialSample.m_MaterialID = surface.m_MaterialID;
        initialSample.m_SamplePosition = payload.m_HitPosition;
        initialSample.m_SampleNormal = payload.m_HitNormal;
        initialSample.m_Radiance = payload.m_Radiance;

        const float3 targetFunction = ComputeTargetFunction(surface, initialSample);
        const float3 risWeight = targetFunction / max(0.001f, pdf);
            
        initialReservoir.Resample(initialSample, Random(screenCoords * g_GlobalConstants.m_FrameNumber), targetFunction, risWeight);
    }

    g_RWOutputReservoir[sampleIdx] = GIReservoir::Pack(initialReservoir);
}

