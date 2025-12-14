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

Texture2D<float2> SceneDepth                        : register(t0);
Texture2D<float4> DirectLightingTexture             : register(t1);
Texture2D<float4> DiffuseIndirectLightingTexture    : register(t2);
Texture2D<float4> ProceduralSkyTexture              : register(t3);

struct PS_INPUT
{
    float4 Position : SV_Position;
    float2 TexCoord : TEXCOORD;
};

float4 PS_Main(PS_INPUT IN) : SV_Target
{
    sampler pointSampler = SamplerDescriptorHeap[GlobalConstants.m_SamplerIndex_Point_Clamp];

    const float4 direct = DirectLightingTexture.Sample(pointSampler, IN.TexCoord);
    const float4 indirect = DiffuseIndirectLightingTexture.Sample(pointSampler, IN.TexCoord);
    const float4 sky = ProceduralSkyTexture[IN.TexCoord * GlobalConstants.m_ScreenResolution];
    const float2 screenCoords = IN.TexCoord * GlobalConstants.m_ScreenResolution;
    const float depth = SceneDepth.Load(int3(screenCoords, 0)).r;
 
    if (depth <= 0) // Reverse-z
        return sky;

    return direct + indirect;
}

#endif // __LIGHTING_COMPOSITE_PS_HLSL__