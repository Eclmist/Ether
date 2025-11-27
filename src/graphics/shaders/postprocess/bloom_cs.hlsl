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

#ifndef __BLOOM_CS_HLSL__
#define __BLOOM_CS_HLSL__

#include "common/globalconstants.h"
#include "common/bloomparams.h"

ConstantBuffer<BloomParams> BloomParams             : register(b1);
Texture2D<float4> SourceTexture                     : register(t0);
Texture2D<float4> DestinationTexture                : register(t1);
RWTexture2D<float4> RWDestinationTexture            : register(u0);

float4 UpsampleSource(uint3 threadID)
{
    sampler linearSampler = SamplerDescriptorHeap[GlobalConstants.m_SamplerIndex_Linear_Clamp];
    const float2 resolution = BloomParams.m_Resolution;
    const float2 halfTexelSize = 1.0f / resolution / 2.0f;
    const float2 uv = threadID.xy / resolution + halfTexelSize;
    const float2 anamorphicFactor = float2(BloomParams.m_Anamorphic, 1);
    const float2 diffractionFactor = float2(1, 1);
    const float offsetConstant = 2;

    float4 offset = float4(-1, -1, 1, 1) 
        * offsetConstant 
        * halfTexelSize.xyxy 
        * anamorphicFactor.xyxy 
        * diffractionFactor.xyxy;


    float4 col = SourceTexture.Sample(linearSampler, uv + float2(offset.x * 2.0f, 0.0f));
    col += SourceTexture.Sample(linearSampler, uv + float2(offset.z * 2.0f, 0.0f));
    col += SourceTexture.Sample(linearSampler, uv + float2(0.0f, offset.y * 2.0f));
    col += SourceTexture.Sample(linearSampler, uv + float2(0.0f, offset.w * 2.0f));

    col += SourceTexture.Sample(linearSampler, uv + (offset.xy)) * 2.0f;
    col += SourceTexture.Sample(linearSampler, uv + (offset.xw)) * 2.0f;
    col += SourceTexture.Sample(linearSampler, uv + (offset.zy)) * 2.0f;
    col += SourceTexture.Sample(linearSampler, uv + (offset.zw)) * 2.0f;

    col /= 12.0f;
    return col;
}

void UpsamplePass(uint3 threadID)
{
    sampler linearSampler = SamplerDescriptorHeap[GlobalConstants.m_SamplerIndex_Linear_Clamp];
    const float2 resolution = BloomParams.m_Resolution;
    const float2 halfTexelSize = 1.0f / resolution / 2.0f;
    const float2 uv = threadID.xy / resolution + halfTexelSize;
    float4 upsampled = UpsampleSource(threadID);
    float4 original = DestinationTexture.Sample(linearSampler, uv);
    RWDestinationTexture[threadID.xy] = lerp(original, upsampled, BloomParams.m_Scatter);
}

void DownsamplePass(uint3 threadID)
{
    sampler linearSampler = SamplerDescriptorHeap[GlobalConstants.m_SamplerIndex_Linear_Clamp];
    const float2 resolution = BloomParams.m_Resolution;
    const float2 halfTexelSize = 1.0f / resolution / 2.0f;
    const float2 uv = threadID.xy / resolution + halfTexelSize;
    const float4 offset = halfTexelSize.xyxy * float4(-1, -1, 1, 1) * 2;

    float4 col = SourceTexture.Sample(linearSampler, uv) * 4.0f;
    col += SourceTexture.Sample(linearSampler, uv + offset.xy);
    col += SourceTexture.Sample(linearSampler, uv + offset.xw);
    col += SourceTexture.Sample(linearSampler, uv + offset.zy);
    col += SourceTexture.Sample(linearSampler, uv + offset.zw);

    RWDestinationTexture[threadID.xy] = col / 8.0f;
}

void CompositePass(uint3 threadID)
{
    sampler linearSampler = SamplerDescriptorHeap[GlobalConstants.m_SamplerIndex_Linear_Clamp];
    const float2 resolution = GlobalConstants.m_ScreenResolution;
    const float2 halfTexelSize = 1.0f / resolution / 2.0f;
    const float2 uv = threadID.xy / resolution + halfTexelSize;
    float4 upsampled = UpsampleSource(threadID);

    const float4 bloom = SourceTexture.Sample(linearSampler, uv);
    RWDestinationTexture[threadID.xy] = lerp(RWDestinationTexture[threadID.xy], upsampled, BloomParams.m_Intensity);
}

[numthreads(BLOOM_KERNEL_GROUP_SIZE_X, BLOOM_KERNEL_GROUP_SIZE_Y, 1)]
void CS_Main(uint3 threadID : SV_DispatchThreadID)
{
    if (any(threadID.xy > BloomParams.m_Resolution.xy))
        return;

    if (BloomParams.m_PassIndex == BLOOM_PASSINDEX_DOWNSAMPLE)
        DownsamplePass(threadID);
    else if (BloomParams.m_PassIndex == BLOOM_PASSINDEX_UPSAMPLE)
        UpsamplePass(threadID);
    else if (BloomParams.m_PassIndex == BLOOM_PASSINDEX_COMPOSITE)
        CompositePass(threadID);
}

#endif // __BLOOM_CS_HLSL__