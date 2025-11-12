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

#ifndef __DEPTH_OF_FIELD_CS_HLSL__
#define __DEPTH_OF_FIELD_CS_HLSL__

#include "common/globalconstants.h"
#include "common/depthoffieldparams.h"
#include "utils/constants.hlsl"
#include "utils/helpers.hlsl"

ConstantBuffer<DepthOfFieldParams> g_DepthOfFieldParams : register(b1);

Texture2D<float4> g_SourceTexture                   : register(t0);
Texture2D<float2> g_SceneDepth                      : register(t1);
Texture2D<float4> g_CircleOfConfusionTexture        : register(t2);
Texture2D<float4> g_DownsampledSceneColor           : register(t3);
Texture2D<float4> g_DofAccumulationTexture          : register(t4);

RWTexture2D<float4> g_DestinationTextureUav         : register(u0);

// From https://github.com/Unity-Technologies/Graphics/blob/master/com.unity.postprocessing/PostProcessing/Shaders/Builtins/DiskKernels.hlsl
static const int g_KernelSampleCount = 71;
static const float2 g_Kernel[g_KernelSampleCount] = {
    float2(0,0),
    float2(0.2758621,0),
    float2(0.1719972,0.21567768),
    float2(-0.061385095,0.26894566),
    float2(-0.24854316,0.1196921),
    float2(-0.24854316,-0.11969208),
    float2(-0.061384983,-0.2689457),
    float2(0.17199717,-0.21567771),
    float2(0.51724136,0),
    float2(0.46601835,0.22442262),
    float2(0.32249472,0.40439558),
    float2(0.11509705,0.50427306),
    float2(-0.11509704,0.50427306),
    float2(-0.3224948,0.40439552),
    float2(-0.46601835,0.22442265),
    float2(-0.51724136,0),
    float2(-0.46601835,-0.22442262),
    float2(-0.32249463,-0.40439564),
    float2(-0.11509683,-0.5042731),
    float2(0.11509732,-0.504273),
    float2(0.32249466,-0.40439564),
    float2(0.46601835,-0.22442262),
    float2(0.7586207,0),
    float2(0.7249173,0.22360738),
    float2(0.6268018,0.4273463),
    float2(0.47299224,0.59311354),
    float2(0.27715522,0.7061801),
    float2(0.056691725,0.75649947),
    float2(-0.168809,0.7396005),
    float2(-0.3793104,0.65698475),
    float2(-0.55610836,0.51599306),
    float2(-0.6834936,0.32915324),
    float2(-0.7501475,0.113066405),
    float2(-0.7501475,-0.11306671),
    float2(-0.6834936,-0.32915318),
    float2(-0.5561083,-0.5159932),
    float2(-0.37931028,-0.6569848),
    float2(-0.16880904,-0.7396005),
    float2(0.056691945,-0.7564994),
    float2(0.2771556,-0.7061799),
    float2(0.47299215,-0.59311366),
    float2(0.62680185,-0.4273462),
    float2(0.72491735,-0.22360711),
    float2(1,0),
    float2(0.9749279,0.22252093),
    float2(0.90096885,0.43388376),
    float2(0.7818315,0.6234898),
    float2(0.6234898,0.7818315),
    float2(0.43388364,0.9009689),
    float2(0.22252098,0.9749279),
    float2(0,1),
    float2(-0.22252095,0.9749279),
    float2(-0.43388385,0.90096885),
    float2(-0.62349,0.7818314),
    float2(-0.7818317,0.62348956),
    float2(-0.90096885,0.43388382),
    float2(-0.9749279,0.22252093),
    float2(-1,0),
    float2(-0.9749279,-0.22252087),
    float2(-0.90096885,-0.43388376),
    float2(-0.7818314,-0.6234899),
    float2(-0.6234896,-0.7818316),
    float2(-0.43388346,-0.900969),
    float2(-0.22252055,-0.974928),
    float2(0,-1),
    float2(0.2225215,-0.9749278),
    float2(0.4338835,-0.90096897),
    float2(0.6234897,-0.7818316),
    float2(0.78183144,-0.62348986),
    float2(0.90096885,-0.43388376),
    float2(0.9749279,-0.22252086),
};

float WeighSample(float coc, float radius)
{
    return saturate((coc - radius + 2.0f) / 2.0f);
}

float WeighSample(float3 c)
{
    return 1.0f / (1.0f + max(max(c.r, c.g), c.b));
}

void GenerateCocPass(uint3 threadID)
{
    const float linearDepth = LinearizeDepth(g_SceneDepth.Load(threadID).r);
    const float coc = clamp((linearDepth - g_DepthOfFieldParams.m_FocusDistance) / (g_DepthOfFieldParams.m_FocusRange), -1.0f, 1.0f) * g_DepthOfFieldParams.m_Aperture;
    g_DestinationTextureUav[threadID.xy] = coc;
}

void PreFilterPass(uint3 threadID)
{
    sampler linearSampler = SamplerDescriptorHeap[g_GlobalConstants.m_SamplerIndex_Linear_Clamp];
    const float2 halfResolution = g_GlobalConstants.m_ScreenResolution / 2.0f;
    const float2 texelSize = 1.0f / halfResolution;
    const float2 halfTexelSize = texelSize / 2.0f;
    const float2 uv = threadID.xy / halfResolution + halfTexelSize;
    const float4 offset = texelSize.xyxy * float2(-0.5f, 0.5f).xxyy;

    const float3 color0 = g_SourceTexture.Sample(linearSampler, uv + offset.xy).xyz;
    const float3 color1 = g_SourceTexture.Sample(linearSampler, uv + offset.zy).xyz;
    const float3 color2 = g_SourceTexture.Sample(linearSampler, uv + offset.xw).xyz;
    const float3 color3 = g_SourceTexture.Sample(linearSampler, uv + offset.zw).xyz;

    const float w0 = WeighSample(color0);
    const float w1 = WeighSample(color1);
    const float w2 = WeighSample(color2);
    const float w3 = WeighSample(color3);

    float3 color = color0 * w0 + color1 * w1 + color2 * w2 + color2 * w3;
    color /= max(w0 + w1 + w2 + w3, 0.0001f);

    const float coc0 = g_CircleOfConfusionTexture.Sample(linearSampler, uv + offset.xy).r;
    const float coc1 = g_CircleOfConfusionTexture.Sample(linearSampler, uv + offset.zy).r;
    const float coc2 = g_CircleOfConfusionTexture.Sample(linearSampler, uv + offset.xw).r;
    const float coc3 = g_CircleOfConfusionTexture.Sample(linearSampler, uv + offset.zw).r;

    const float cocMin = min(min(min(coc0, coc1), coc2), coc3);
    const float cocMax = max(max(max(coc0, coc1), coc2), coc3);
    const float coc = cocMax >= -cocMin ? cocMax : cocMin;

    g_DestinationTextureUav[threadID.xy] = float4(color, coc);
}

void AccumulateDepthOfField(uint3 threadID)
{
    sampler pointSampler = SamplerDescriptorHeap[g_GlobalConstants.m_SamplerIndex_Point_Clamp];
    sampler linearSampler = SamplerDescriptorHeap[g_GlobalConstants.m_SamplerIndex_Linear_Clamp];
    const float2 halfResolution = g_GlobalConstants.m_ScreenResolution / 2.0f;
    const float2 texelSize = 1.0f / halfResolution;
    const float2 halfTexelSize = texelSize / 2.0f;
    const float2 uv = threadID.xy / halfResolution + halfTexelSize;
    const float coc = g_SourceTexture.Sample(pointSampler, uv).w;

    float3 backgroundColor = 0;
    float3 foregroundColor = 0;
    float backgroundWeight = 0;
    float foregroundWeight = 0;

    for (int i = 0; i < g_KernelSampleCount; ++i)
    {
        float2 offset = g_Kernel[i] * g_DepthOfFieldParams.m_Aperture;
        float radius = length(offset);
        offset *= texelSize;
        float4 sample = g_SourceTexture.Sample(pointSampler, uv + offset);

        float backgroundSampleWeight = WeighSample(max(0, sample.w), radius);
        backgroundColor += sample.rgb * backgroundSampleWeight;
        backgroundWeight += backgroundSampleWeight;

        float foregroundSampleWeight = WeighSample(-sample.w, radius);
        foregroundColor += sample.rgb * foregroundSampleWeight;
        foregroundWeight += foregroundSampleWeight;
    }

    backgroundColor *= 1.0f / (backgroundWeight + (backgroundWeight == 0));
    foregroundColor *= 1.0f / (foregroundWeight + (foregroundWeight == 0));

    float blendFactor = min(1.0f, foregroundWeight * Pi / g_KernelSampleCount);
    float3 finalColor = lerp(backgroundColor, foregroundColor, blendFactor);
    g_DestinationTextureUav[threadID.xy] = float4(finalColor, blendFactor);
}

void PostFilterPass(uint3 threadID)
{
    sampler linearSampler = SamplerDescriptorHeap[g_GlobalConstants.m_SamplerIndex_Linear_Clamp];
    const float2 halfResolution = g_GlobalConstants.m_ScreenResolution / 2.0f;
    const float2 texelSize = 1.0f / halfResolution;
    const float2 halfTexelSize = texelSize / 2.0f;
    const float2 uv = threadID.xy / halfResolution + halfTexelSize;

    float4 offset = texelSize.xyxy * float2(-0.5f, 0.5f).xxyy;
    float4 tentFilter = g_SourceTexture.Sample(linearSampler, uv + offset.xy) + 
                        g_SourceTexture.Sample(linearSampler, uv + offset.zy) + 
                        g_SourceTexture.Sample(linearSampler, uv + offset.xw) + 
                        g_SourceTexture.Sample(linearSampler, uv + offset.zw);

    g_DestinationTextureUav[threadID.xy] = tentFilter * 0.25f;
}

void CompositePass(uint3 threadID)
{
    sampler linearSampler = SamplerDescriptorHeap[g_GlobalConstants.m_SamplerIndex_Linear_Clamp];
    const float2 resolution = g_GlobalConstants.m_ScreenResolution;
    const float2 texelSize = 1.0f / resolution;
    const float2 halfTexelSize = texelSize / 2.0f;
    const float2 uv = threadID.xy / resolution + halfTexelSize;

    const float coc = g_CircleOfConfusionTexture.Load(threadID).r;
    const float4 sceneColor = g_SourceTexture.Sample(linearSampler, uv);
    const float4 dofColor = g_DofAccumulationTexture.Sample(linearSampler, uv);

    // Debug visualizer
    if (g_GlobalConstants.m_RaytracedLightingDebug == 1)
    {
        float4 debugColor = float4(0, 1, 0, 1);
        if (coc > 0)
            debugColor = float4(0, 0, 1, 1);

        g_DestinationTextureUav[threadID.xy] = lerp(sceneColor, sceneColor * debugColor, saturate(abs(coc)));
        return;
    }

    const float dofStrength = smoothstep(0.1, 1, abs(coc));
    const float3 finalColor = lerp(sceneColor, dofColor, dofStrength + dofColor.a - dofStrength * dofColor.a).rgb;

    g_DestinationTextureUav[threadID.xy] = float4(finalColor, sceneColor.a);
}

[numthreads(DOF_KERNEL_GROUP_SIZE_X, DOF_KERNEL_GROUP_SIZE_Y, 1)]
void CS_Main(uint3 threadID : SV_DispatchThreadID)
{
    if (any(threadID.xy > g_GlobalConstants.m_ScreenResolution))
        return;

    if (g_DepthOfFieldParams.m_PassIndex == DOF_PASSINDEX_GENERATE_COC)
        GenerateCocPass(threadID);
    else if (g_DepthOfFieldParams.m_PassIndex == DOF_PASSINDEX_PREFILTER_PASS)
        PreFilterPass(threadID);
    else if (g_DepthOfFieldParams.m_PassIndex == DOF_PASSINDEX_ACCUMULATE)
        AccumulateDepthOfField(threadID);
    else if (g_DepthOfFieldParams.m_PassIndex == DOF_PASSINDEX_POSTFILTER_PASS)
        PostFilterPass(threadID);
    else if (g_DepthOfFieldParams.m_PassIndex == DOF_PASSINDEX_COMPOSITE)
        CompositePass(threadID);
}

#endif // __DEPTH_OF_FIELD_CS_HLSL__
