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

#ifndef __TEMPORAL_AA_CS_HLSL
#define __TEMPORAL_AA_CS_HLSL

#include "common/globalconstants.h"
#include "utils/fullscreenhelpers.hlsl"

Texture2D<float4> g_GBufferTexture1                 : register(t0);
Texture2D<float4> g_AccumulationTextureIn           : register(t1);
RWTexture2D<float4> g_TargetTexture                 : register(u0);
RWTexture2D<float4> g_AccumulationTextureOut        : register(u1);

[numthreads(32, 32, 1)]
void CS_Main(uint3 threadID : SV_DispatchThreadID)
{
    sampler pointSampler = SamplerDescriptorHeap[g_GlobalConstants.m_SamplerIndex_Point_Clamp];
    sampler linearSampler = SamplerDescriptorHeap[g_GlobalConstants.m_SamplerIndex_Linear_Clamp];
    const float2 resolution = g_GlobalConstants.m_ScreenResolution;
    const float2 screenCoords = threadID.xy;
    const float2 velocity = g_GBufferTexture1.Load(threadID).zw;
    const float2 uv = ScreenToTextureSpace(screenCoords);
    const float2 uvPrev = uv - velocity;

    if (any(threadID.xy < 0) || any(threadID.xy >= g_GlobalConstants.m_ScreenResolution.xy))
        return;

    if (uvPrev.x < 0 || uvPrev.y < 0 || uvPrev.x >= 1.0f || uvPrev.y >= 1.0f)
        return;

    const float4 colorPrev = g_AccumulationTextureIn.SampleLevel(linearSampler, uvPrev, 0);
    const float4 colorCurr = g_TargetTexture[threadID.xy];

    // Variance Clipping
    float4 minColor = 9999999.0, maxColor = -9999999.0;
    const int kernelSize = 1;
    for (int x = -kernelSize; x <= kernelSize; ++x)
    {
        for (int y = -kernelSize; y <= kernelSize; ++y)
        {
            float4 color = g_TargetTexture[threadID.xy + int2(x, y)];
            minColor = min(minColor, color);
            maxColor = max(maxColor, color);
        }
    }

    const float a = g_GlobalConstants.m_TaaAccumulationFactor;
    const float4 previousColorClamped = clamp(colorPrev, minColor, maxColor);
    const float4 newColor = (a * colorCurr) + (1 - a) * previousColorClamped;

    g_TargetTexture[threadID.xy] = newColor;
}

#endif // __TEMPORAL_AA_CS_HLSL
