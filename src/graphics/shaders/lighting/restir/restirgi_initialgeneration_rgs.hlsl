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

#ifndef __RESTIR_GI_INITIAL_GENERATION_RGS_HLSL__
#define __RESTIR_GI_INITIAL_GENERATION_RGS_HLSL__

#include "lighting/restir/gireservoirresampling.hlsl"
#include "utils/helpers.hlsl"

[shader("raygeneration")]
void RayGeneration()
{
    const uint2 sampleCoords = DispatchRaysIndex().xy;
    const uint2 screenCoords = GetScreenCoordsFromSampleCoords(sampleCoords);
    const uint2 bufferSize = DispatchRaysDimensions().xy;
    const uint sampleIdx = GetSampleIndexFromSampleCoords(sampleCoords, bufferSize);

    if (any(screenCoords >= GlobalConstants.m_ScreenResolution.xy))
        return;

    const ShadingSurface surface = GetShadingSurfaceFromGBuffers(screenCoords, GBufferTextureA, GBufferTextureB, GBufferTextureC, SceneDepth);
    const float depth = SceneDepth.Load(int3(screenCoords, 0)).r;

    if (depth <= 0) // Reverse-z
        return;

    const float3 wo = normalize(GlobalConstants.m_CameraPosition.xyz - surface.m_Position);
    float3 wi;
    float pdf;

#if USE_IMPORTANCE_SAMPLING
    SampleDirectionBrdf(surface, GlobalConstants.m_FrameNumber, wo, wi, pdf);
#else
    SampleDirectionUniform(surface, GlobalConstants.m_FrameNumber, wi, pdf);
#endif

    GIReservoir initialReservoir = GIReservoir::Empty();
    GIReservoirSample initialSample = GIReservoirSample::Empty();

    const RayPayload payload = TraceShadingRay(surface, wi, MAX_DEPTH);
    initialSample.m_VisibleDepth = LinearizeDepth(depth);
    initialSample.m_VisibleNormal = surface.m_Normal;
    initialSample.m_MaterialID = surface.m_MaterialID;
    initialSample.m_SamplePosition = payload.m_HitPosition;
    initialSample.m_SampleNormal = payload.m_HitNormal;
    initialSample.m_Radiance = payload.m_Radiance;

    const float3 targetFunction = ComputeTargetFunction(surface, initialSample);
    const float3 risWeight = targetFunction / max(0.05f, pdf);
        
    initialReservoir.Resample(initialSample, Random(screenCoords * GlobalConstants.m_FrameNumber), targetFunction, risWeight);

    RWOutputReservoir[sampleIdx] = GIReservoir::Pack(initialReservoir);

    // Spatial hash prototype
    uint cellIndex = SpatialHash_FindOrInsert(surface.m_Position, surface.m_Normal);

    if (cellIndex != 0xFFFFFFFFu)
    {
        initialReservoir.FinalizeResampling();
        SpatialHashPayload payload;
        payload.m_Color = surface.m_Normal;
        RWSpatialHashPayload[cellIndex] = payload;
    }
}

#endif // __RESTIR_INITIAL_GENERATION_RGS_HLSL__
