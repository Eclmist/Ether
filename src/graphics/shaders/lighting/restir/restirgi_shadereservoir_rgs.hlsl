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

RWTexture2D<float4> g_LightingOutput                        : register(u3);

[shader("raygeneration")]
void RayGeneration()
{
    const uint2 screenSize = DispatchRaysDimensions().xy;
    const uint2 screenCoords = DispatchRaysIndex().xy;
    uint sampleIdx = GetSampleIndexFromScreenCoords(screenCoords, screenSize);

    if (any(screenCoords < 0) || any(screenCoords >= g_GlobalConstants.m_ScreenResolution.xy))
        return;

#if DOWNSAMPLE_FACTOR != 1
    const float2 stochasticOffsets = (CMJ_Sample2D(sampleIdx, 1024, 1024, g_GlobalConstants.m_FrameNumber + 400.0f) - 0.5f) * 2.0f;
    const uint2 sampleCoords = round( GetSampleCoordsFromScreenCoords(screenCoords) + stochasticOffsets);
    sampleIdx = GetSampleIndexFromSampleCoords(sampleCoords, screenSize / (float)DOWNSAMPLE_FACTOR);
#endif

    ShadingSurface surface = GetShadingSurfaceFromGBuffers(screenCoords, g_GBufferA, g_GBufferB, g_GBufferC, g_SceneDepth);
    GIReservoir finalReservoir = GIReservoir::Unpack(g_InputReservoir[sampleIdx]);
    DeviceMemoryBarrier();

    const RayPayload shadowRay = TraceShadowRay(surface, g_GlobalConstants.m_SunDirection.xyz);
    const float3 Li = shadowRay.m_Radiance;
    const float3 wi = normalize(g_GlobalConstants.m_SunDirection.xyz);
    const float3 wo = normalize(g_GlobalConstants.m_CameraPosition.xyz - surface.m_Position);
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
        //    g_RWOutputReservoir[sampleIdx] = GIReservoir::Pack(finalReservoir);
        //}
        
        indirectLighting = finalReservoir.m_TargetPdf * finalReservoir.m_WeightSum;
    }

    // Spatial Hash Prototype
    if (g_GlobalConstants.m_RaytracedLightingDebug == 1)
    {

        const float cellSize = g_GlobalConstants.m_SpatialHashCellSize;
		uint cellIndex = SpatialHash_Lookup(surface.m_Position, surface.m_Normal);

        float3 finalColor = float3(1, 0, 1);

		if (cellIndex != 0xFFFFFFFFu)
		{
			finalColor = g_SpatialHashPayload[cellIndex].m_Color;
		}

		g_LightingOutput[screenCoords].xyz = finalColor * 1;
		g_LightingOutput[screenCoords].a = 0;
        return;
    }


    g_LightingOutput[screenCoords].xyz = surface.m_Emission + directLighting + indirectLighting;
    g_LightingOutput[screenCoords].a = 0;
}

#endif // __RESTIR_GI_SHADE_RESERVOIR_RGS_HLSL__
