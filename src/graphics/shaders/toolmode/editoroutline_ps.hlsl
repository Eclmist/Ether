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

#ifndef __EDITOR_OUTLINE_PS_HLSL__
#define __EDITOR_OUTLINE_PS_HLSL__

#include "common/globalconstants.h"
#include "utils/constants.hlsl"
#include "utils/helpers.hlsl"

#define OUTLINE_WIDTH               2
#define OUTLINE_COLOR               float4(1, 0.35, 0.1, 1.0)

// The area color effect looks nice and makes the selection very obvious
// but it also means that it's harder to adjust materials and visually inspect
// a selected object due to the overlay :( 
// Disable for now.
#define USE_OVERLAY_COLOR 0
#define OUTLINE_AREA_COLOR          float4(1, 0.35, 0.1, 0.15)

// A different color can be set for occluded pixels, but it makes the
// selection look quite messy so disabling for now.
// It can also be set to black for alpha tested outlines
#define USE_OCCLUDED_COLOR 0
#define OUTLINE_AREA_OCCLUDED_COLOR float4(1, 0.25, 0.1, 0.05)

Texture2D<float2> SceneDepth : register(t0);
Texture2D<float4> OutlineMaskTexture : register(t1);

float4 PS_OutlineMask(float4 SVPosition : SV_Position) : SV_Target
{
    const float pixelDepth = LinearizeDepth(SVPosition.z);
    const float sceneDepth = LinearizeDepth(SceneDepth.Load(float3(SVPosition.xy, 0)).r);

#if USE_OCCLUDED_COLOR
    if (sceneDepth - pixelDepth < -0.01)
        return OUTLINE_AREA_OCCLUDED_COLOR;
    else
#endif
        return OUTLINE_AREA_COLOR;
}

float4 PS_Main(float4 SVPosition : SV_Position) : SV_Target
{
    float4 outlineMask = OutlineMaskTexture.Load(float3(SVPosition.xy, 0));

    if (all(outlineMask) != 0)
#if USE_OVERLAY_COLOR
        return outlineMask;
#else
        discard;
#endif

    for (int u = -OUTLINE_WIDTH; u <= OUTLINE_WIDTH; ++u)
    {
        for (int v = -OUTLINE_WIDTH; v <= OUTLINE_WIDTH; ++v)
        {
            float4 neighbour = OutlineMaskTexture.Load(float3(SVPosition.xy + float2(u, v), 0));

            if (any(neighbour) != 0)
            {
                return OUTLINE_COLOR;
            }
        }
    }

    discard;
    return 0;
}

#endif // __EDITOR_OUTLINE_PS_HLSL__