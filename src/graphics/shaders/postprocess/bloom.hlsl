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
#include "common/bloomparams.h"

ConstantBuffer<GlobalConstants> g_GlobalConstants   : register(b0);
ConstantBuffer<BloomParams> g_BloomParams           : register(b1);
Texture2D<float4> g_SourceTexture                   : register(t0);
RWTexture2D<float4> g_DestinationTexture            : register(u0);

void DownsamplePass(uint3 threadID)
{
    sampler linearSampler = SamplerDescriptorHeap[g_GlobalConstants.m_SamplerIndex_Linear_Clamp];
    const float2 resolution = g_BloomParams.m_Resolution;
    const float2 halfTexelSize = 1.0f / resolution / 2.0f;
    const float2 uv = threadID.xy / resolution + halfTexelSize;
    const float4 offset = halfTexelSize.xyxy * float4(-1, -1, 1, 1);

    float4 col = g_SourceTexture.Sample(linearSampler, uv) * 4.0f;
    col += g_SourceTexture.Sample(linearSampler, uv + offset.xy);
    col += g_SourceTexture.Sample(linearSampler, uv + offset.xw);
    col += g_SourceTexture.Sample(linearSampler, uv + offset.zy);
    col += g_SourceTexture.Sample(linearSampler, uv + offset.zw);

    g_DestinationTexture[threadID.xy] = col / 8.0f;
}

void UpsamplePass(uint3 threadID)
{
    sampler linearSampler = SamplerDescriptorHeap[g_GlobalConstants.m_SamplerIndex_Linear_Clamp];
    const float2 resolution = g_BloomParams.m_Resolution;
    const float2 halfTexelSize = 1.0f / resolution / 2.0f;
    const float2 uv = threadID.xy / resolution + halfTexelSize;
    const float4 offset = halfTexelSize.xyxy * float4(-1, -1, 1, 1);
    const float2 anamorphy = float2(1.0f + (g_BloomParams.m_Anamorphic), 1.0f);

    float4 col = g_SourceTexture.Sample(linearSampler, uv + float2(offset.x, 0.0f) * anamorphy);
    col += g_SourceTexture.Sample(linearSampler, uv + float2(offset.z, 0.0f) * anamorphy);
    col += g_SourceTexture.Sample(linearSampler, uv + float2(0.0f, offset.y) * anamorphy);
    col += g_SourceTexture.Sample(linearSampler, uv + float2(0.0f, offset.w) * anamorphy);

    col += g_SourceTexture.Sample(linearSampler, uv + (offset.xy / 2.0f) * anamorphy) * 2.0f;
    col += g_SourceTexture.Sample(linearSampler, uv + (offset.xw / 2.0f) * anamorphy) * 2.0f;
    col += g_SourceTexture.Sample(linearSampler, uv + (offset.zy / 2.0f) * anamorphy) * 2.0f;
    col += g_SourceTexture.Sample(linearSampler, uv + (offset.zw / 2.0f) * anamorphy) * 2.0f;

    g_DestinationTexture[threadID.xy] = col / 12.0f;
}

void CompositePass(uint3 threadID)
{
    const float2 resolution = g_GlobalConstants.m_ScreenResolution;
    const float2 halfPixel = 1.0f / resolution / 2.0f;
    const float2 uv = threadID.xy / resolution + halfPixel;
    const float bloomIntensity = g_BloomParams.m_Intensity * 0.82;

    sampler linearSampler = SamplerDescriptorHeap[g_GlobalConstants.m_SamplerIndex_Linear_Clamp];
    const float4 bloom = g_SourceTexture.Sample(linearSampler, uv);
    g_DestinationTexture[threadID.xy] += bloom * bloomIntensity;
}

[numthreads(BLOOM_KERNEL_GROUP_SIZE_X, BLOOM_KERNEL_GROUP_SIZE_Y, 1)]
void CS_Main(uint3 threadID : SV_DispatchThreadID)
{
    //if (threadID.y > g_BloomParams.m_Resolution.y)
    //    return;
    if (g_BloomParams.m_PassIndex == BLOOM_PASSINDEX_DOWNSAMPLE)
        DownsamplePass(threadID);
    else if (g_BloomParams.m_PassIndex == BLOOM_PASSINDEX_UPSAMPLE)
        UpsamplePass(threadID);
    else if (g_BloomParams.m_PassIndex == BLOOM_PASSINDEX_COMPOSITE)
        CompositePass(threadID);
}