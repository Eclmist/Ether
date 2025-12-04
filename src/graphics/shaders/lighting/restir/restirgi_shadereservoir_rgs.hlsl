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

#ifndef __RESTIR_GI_SHADE_RESERVOIR_RGS_HLSL__
#define __RESTIR_GI_SHADE_RESERVOIR_RGS_HLSL__

#include "lighting/restir/gireservoirresampling.hlsl"

RWTexture2D<float4> RWLightingTexture                   : register(u3);

[shader("raygeneration")]
void RayGeneration()
{
    const uint2 screenSize = DispatchRaysDimensions().xy;
    const uint2 screenCoords = DispatchRaysIndex().xy;
    uint sampleIdx = GetSampleIndexFromScreenCoords(screenCoords, screenSize);

    if (any(screenCoords < 0) || any(screenCoords >= GlobalConstants.m_ScreenResolution.xy))
        return;

#if DOWNSAMPLE_FACTOR != 1
    const float2 stochasticOffsets = (CMJ_Sample2D(sampleIdx, 1024, 1024, GlobalConstants.m_FrameNumber + 400.0f) - 0.5f) * 2.0f;
    const uint2 sampleCoords = round( GetSampleCoordsFromScreenCoords(screenCoords) + stochasticOffsets);
    sampleIdx = GetSampleIndexFromSampleCoords(sampleCoords, screenSize / (float)DOWNSAMPLE_FACTOR);
#endif

    const ShadingSurface surface = GetShadingSurfaceFromGBuffers(screenCoords, GBufferTextureA, GBufferTextureB, GBufferTextureC, SceneDepth);
    GIReservoir finalReservoir = GIReservoir::Unpack(InputReservoir[sampleIdx]);
    DeviceMemoryBarrier();

    const RayPayload shadowRay = TraceShadowRay(surface, GlobalConstants.m_SunDirection.xyz);
    const float3 Li = shadowRay.m_Radiance;
    const float3 wi = normalize(GlobalConstants.m_SunDirection.xyz);
    const float3 wo = normalize(GlobalConstants.m_CameraPosition.xyz - surface.m_Position);
    const float3 directLighting = ComputeRadiance(surface, Li, wi, wo);
    float3 indirectLighting = 0;

    finalReservoir.m_TargetPdf = ComputeTargetFunction(surface, finalReservoir.m_Sample);
    finalReservoir.FinalizeResampling();

    if (finalReservoir.IsValid())
    {
        //const RayPayload validationRay = TraceValidationRay(surface, finalReservoir.m_Sample);

        //if (validationRay.m_Hit)
        //{
        //    finalReservoir = GIReservoir::Empty();
        //    RWOutputReservoir[sampleIdx] = GIReservoir::Pack(finalReservoir);
        //}
        
        indirectLighting = finalReservoir.m_TargetPdf * finalReservoir.m_WeightSum;
    }

    // Spatial Hash Prototype
    if (GlobalConstants.m_RaytracedLightingDebug == 1)
    {

		uint cellIndex = SpatialHash_Lookup(surface.m_Position, surface.m_Normal);

        float3 finalColor = float3(1, 0, 1) * 1000;

		if (cellIndex != 0xFFFFFFFFu)
		{
            float numSamples = RWSpatialHashPayload[cellIndex].m_NumSamples;
		    finalColor.x = RWSpatialHashPayload[cellIndex].m_Radiance.x / numSamples;
		    finalColor.y = RWSpatialHashPayload[cellIndex].m_Radiance.y / numSamples;
		    finalColor.z = RWSpatialHashPayload[cellIndex].m_Radiance.z / numSamples;
		}

		RWLightingTexture[screenCoords].xyz = finalColor * surface.m_BaseColor / Pi;
		RWLightingTexture[screenCoords].a = 0;
        return;
    }


    RWLightingTexture[screenCoords].xyz = surface.m_Emission + directLighting + indirectLighting;
    RWLightingTexture[screenCoords].a = 0;
}

#endif // __RESTIR_GI_SHADE_RESERVOIR_RGS_HLSL__
