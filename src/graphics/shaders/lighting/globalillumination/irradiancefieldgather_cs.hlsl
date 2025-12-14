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

#ifndef __IRRADIANCE_FIELD_GATHER_CS_HLSL__
#define __IRRADIANCE_FIELD_GATHER_CS_HLSL__

#include "common/globalconstants.h"
#include "lighting/globalillumination/irradiancefield.hlsl"
#include "utils/shading.hlsl"

RWTexture2D<float4> RWDiffuseIndirectLightingTexture : register(u0);

Texture2D<float4> GBufferTextureA                    : register(t4);
Texture2D<float4> GBufferTextureB                    : register(t5);
Texture2D<float4> GBufferTextureC                    : register(t6);
Texture2D<float2> SceneDepth                         : register(t7);

[numthreads(32, 32, 1)]
void CS_Main(uint3 threadID : SV_DispatchThreadID)
{
    const uint2 screenCoords = threadID.xy;

    if (any(screenCoords < 0) || any(screenCoords >= GlobalConstants.m_ScreenResolution.xy))
        return;

    const ShadingSurface surface = GetShadingSurfaceFromGBuffers(screenCoords, GBufferTextureA, GBufferTextureB, GBufferTextureC, SceneDepth);
    const float3 indirect = SampleIrradianceField(surface.m_Position, surface.m_Normal);
    RWDiffuseIndirectLightingTexture[screenCoords] = float4(indirect * surface.m_BaseColor / Pi, 1.0f);
}

#endif // __IRRADIANCE_FIELD_GATHER_CS_HLSL__
