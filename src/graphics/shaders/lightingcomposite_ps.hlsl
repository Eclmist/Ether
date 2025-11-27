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

#ifndef __LIGHTING_COMPOSITE_PS_HLSL__
#define __LIGHTING_COMPOSITE_PS_HLSL__

#include "common/globalconstants.h"
#include "utils/encoding.hlsl"
#include "utils/shading.hlsl"

Texture2D<float4> GBufferTextureA           : register(t0);
Texture2D<float4> GBufferTextureB           : register(t1);
Texture2D<float4> GBufferTextureC           : register(t2);
Texture2D<float2> SceneDepth                : register(t3);
Texture2D<float4> LightingTexture           : register(t4);
Texture2D<float4> ProceduralSkyTexture      : register(t5);

struct PS_INPUT
{
    float4 Position : SV_Position;
    float2 TexCoord : TEXCOORD;
};

float4 PS_Main(PS_INPUT IN) : SV_Target
{
    sampler linearSampler = SamplerDescriptorHeap[GlobalConstants.m_SamplerIndex_Linear_Wrap];

    const float4 lighting = LightingTexture.Sample(linearSampler, IN.TexCoord);
    const float4 sky = ProceduralSkyTexture[IN.TexCoord * GlobalConstants.m_ScreenResolution];
    const float2 screenCoords = IN.TexCoord * GlobalConstants.m_ScreenResolution;
    const float depth = SceneDepth.Load(int3(screenCoords, 0)).r;
 
    const ShadingSurface surface = GetShadingSurfaceFromGBuffers(screenCoords, GBufferTextureA, GBufferTextureB, GBufferTextureC, SceneDepth);

    // Hack to get sky which is basically nothing drawn in gbuffer
    if (depth <= 0) // Reverse-z
        return sky;

    // Debug: 
    if (GlobalConstants.m_RaytracedLightingDebug == 1)
        return float4(surface.m_Normal, 0.0f) * 10000.0f;
    //if (GlobalConstants.m_RaytracedLightingDebug == 1)
    //    return float4(surface.m_Position, 1.0f);

    float4 finalColor = lighting;
    return finalColor;
}

#endif // __LIGHTING_COMPOSITE_PS_HLSL__