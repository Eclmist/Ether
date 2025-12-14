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

#ifndef __DIRECT_LIGHTING_RGS_HLSL__
#define __DIRECT_LIGHTING_RGS_HLSL__

#include "utils/sampling.hlsl"
#include "utils/raytracing.hlsl"
#include "utils/helpers.hlsl"
#include "common/raytracingconstants.h"
#include "common/material.h"

Texture2D<float2> SceneDepth                                : register(t3);
Texture2D<float4> GBufferTextureA                           : register(t4);
Texture2D<float4> GBufferTextureB                           : register(t5);
Texture2D<float4> GBufferTextureC                           : register(t6);

RWTexture2D<float4> RWDirectLightingTexture                 : register(u0);

[shader("raygeneration")]
void RayGeneration()
{
    const uint2 screenSize = DispatchRaysDimensions().xy;
    const uint2 screenCoords = DispatchRaysIndex().xy;

    if (any(screenCoords < 0) || any(screenCoords >= GlobalConstants.m_ScreenResolution.xy))
        return;

    const ShadingSurface surface = GetShadingSurfaceFromGBuffers(screenCoords, GBufferTextureA, GBufferTextureB, GBufferTextureC, SceneDepth);
    const RayPayload shadowRay = TraceShadowRay(surface, GlobalConstants.m_SunDirection.xyz);
    const float3 Li = shadowRay.m_Radiance;
    const float3 wi = normalize(GlobalConstants.m_SunDirection.xyz);
    const float3 wo = normalize(GlobalConstants.m_CameraPosition.xyz - surface.m_Position);
    float3 Lo = ComputeRadiance(surface, Li, wi, wo);

    if (GlobalConstants.m_RaytracedLightingDebug)
        Lo = Li;

    RWDirectLightingTexture[screenCoords].xyz = Lo;
}

#endif // __DIRECT_LIGHTING_RGS_HLSL__
