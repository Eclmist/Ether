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

#include "common/globalconstants.h"
#include "utils/fullscreenhelpers.hlsl"

ConstantBuffer<GlobalConstants> g_GlobalConstants   : register(b0);
Texture2D<float4> g_GBufferTexture3                 : register(t0);
Texture2D<float4> g_AccumulationTextureIn           : register(t1);
RWTexture2D<float4> g_TargetTexture                 : register(u0);
RWTexture2D<float4> g_AccumulationTextureOut        : register(u1);

[numthreads(32, 32, 1)]
void CS_Main(uint3 threadID : SV_DispatchThreadID)
{
    sampler pointSampler = SamplerDescriptorHeap[g_GlobalConstants.m_SamplerIndex_Point_Clamp];
    sampler linearSampler = SamplerDescriptorHeap[g_GlobalConstants.m_SamplerIndex_Linear_Clamp];
    float2 resolution = g_GlobalConstants.m_ScreenResolution;
    float2 uv = threadID.xy / resolution + 0.5 / resolution;
    float2 uvPrev = uv - (float2)g_GBufferTexture3.Sample(linearSampler, uv).xy;
    float4 colorPrev = g_AccumulationTextureIn.Sample(linearSampler, uvPrev);
    float4 colorCurr = g_TargetTexture[threadID.xy];

    // Variance Clipping
    float4 minColor = 9999.0, maxColor = -9999.0;
    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            float4 color = g_TargetTexture[threadID.xy + int2(x, y)];
            minColor = min(minColor, color);
            maxColor = max(maxColor, color);
        }
    }
    float4 previousColorClamped = clamp(colorPrev, minColor, maxColor);

    float a = g_GlobalConstants.m_TaaAccumulationFactor;
    float4 newColor = (a * colorCurr) + (1 - a) * previousColorClamped;

    g_TargetTexture[threadID.xy] = newColor;
    //g_TargetTexture[threadID.xy] = float4(uvPrev, 0,0);
}