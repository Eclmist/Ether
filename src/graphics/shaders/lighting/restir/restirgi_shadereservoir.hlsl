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

RWTexture2D<float4> g_LightingOutput                        : register(u3);

[shader("raygeneration")]
void RayGeneration()
{
    const uint2 screenCoords = DispatchRaysIndex().xy;
    const uint3 screenDims = DispatchRaysDimensions();
    const uint sampleIdx = screenCoords.y * screenDims.x + screenCoords.x;

    const ShadingSurface surface = GetShadingSurfaceFromGBuffers(screenCoords, g_GBufferA, g_GBufferB, g_GBufferC, g_GBufferD);
    const GIReservoir finalReservoir = GIReservoir::Unpack(g_InputReservoir[sampleIdx]);

    const RayPayload shadowRay = TraceShadowRay(surface);
    const float3 Li = shadowRay.m_Radiance;
    const float3 wo = normalize(g_GlobalConstants.m_CameraPosition.xyz - surface.m_Position);
    const float3 wi = normalize(g_GlobalConstants.m_SunDirection.xyz);
    const float3 f = BRDF_UE4(wi, wo, surface.m_Normal, surface.m_Albedo, surface.m_Roughness, surface.m_Metalness);
    const float cosTheta = saturate(dot(wi, surface.m_Normal));
    const float3 directLighting = f * Li * cosTheta;

    finalReservoir.FinalizeResampling();
    g_LightingOutput[screenCoords].xyz = directLighting + ComputeRadiance(surface, finalReservoir.m_Sample.m_Radiance, wi, wo) * finalReservoir.m_WeightSum;
    g_LightingOutput[screenCoords].a = 0;
}

[shader("miss")]
void Miss(inout RayPayload payload)
{
    payload.m_Hit = false;

    // Sample sun color
    const float lerpFactor = saturate(dot(g_GlobalConstants.m_SunDirection.xyz, float3(0, 1, 0)));
    payload.m_Radiance = lerp(0.0f, g_GlobalConstants.m_SunColor.xyz, lerpFactor);
}

[shader("closesthit")]
void ClosestHit(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attribs)
{
    payload.m_Hit = true;
    payload.m_Radiance = 0;
}
