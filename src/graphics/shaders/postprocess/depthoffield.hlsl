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
#include "common/depthoffieldparams.h"

ConstantBuffer<GlobalConstants> g_GlobalConstants   : register(b0);
ConstantBuffer<DepthOfFieldParams> g_DepthOfFieldParams : register(b1);

Texture2D<float4> g_SourceTexture                   : register(t0);
Texture2D<float4> g_GBufferTexture3                 : register(t1);
Texture2D<float4> g_BloomTexture1                   : register(t2);
Texture2D<float4> g_BloomTexture2                   : register(t3);
Texture2D<float4> g_BloomTexture3                   : register(t4);
Texture2D<float4> g_BloomTexture4                   : register(t5);
Texture2D<float4> g_BloomTexture5                   : register(t6);
Texture2D<float4> g_BloomTexture6                   : register(t7);
Texture2D<float4> g_BloomTexture7                   : register(t8);
Texture2D<float4> g_BloomTexture8                   : register(t9);

RWTexture2D<float4> g_DestinationTextureUav         : register(u0);

float CoC(float linearDepth)
{
    float z = linearDepth; 
    float f = g_DepthOfFieldParams.m_FocalLength;

    float N = g_DepthOfFieldParams.m_Aperture;
    float focusDist = g_DepthOfFieldParams.m_FocusDistance;

    // thin lens formula for CoC in scene units
    float coc = abs((z - focusDist) / z) * (f * f / (N * (focusDist - f)));

    return coc;
}

int GetBloomMipLevel(float coc)
{
    float maxCoC = g_DepthOfFieldParams.m_MaxCoC;
    float normalized = saturate(coc / maxCoC);
    int mip = (int)(normalized * 8.0);
    return mip;
}

float4 SampleBloomMip(int mip, float2 uv, sampler linearSampler)
{
    if (mip == 0) return g_SourceTexture.Sample(linearSampler, uv);
    else if (mip == 1) return g_BloomTexture1.Sample(linearSampler, uv);
    else if (mip == 2) return g_BloomTexture2.Sample(linearSampler, uv);
    else if (mip == 3) return g_BloomTexture3.Sample(linearSampler, uv);
    else if (mip == 4) return g_BloomTexture4.Sample(linearSampler, uv);
    else if (mip == 5) return g_BloomTexture5.Sample(linearSampler, uv);
    else if (mip == 6) return g_BloomTexture6.Sample(linearSampler, uv);
    else if (mip == 7) return g_BloomTexture7.Sample(linearSampler, uv);
    else return g_BloomTexture8.Sample(linearSampler, uv);
}

[numthreads(DOF_KERNEL_GROUP_SIZE_X, DOF_KERNEL_GROUP_SIZE_Y, 1)]
void CS_Main(uint3 threadID : SV_DispatchThreadID)
{
    if (any(threadID.xy > g_GlobalConstants.m_ScreenResolution))
        return;

    sampler linearSampler = SamplerDescriptorHeap[g_GlobalConstants.m_SamplerIndex_Linear_Clamp];
    const float2 resolution = g_GlobalConstants.m_ScreenResolution;
    const float2 halfTexelSize = 1.0f / resolution / 2.0f;
    const float2 uv = threadID.xy / resolution + halfTexelSize;

    float4 color = g_SourceTexture.Sample(linearSampler, uv);
    float linearDepth = g_GBufferTexture3.Sample(linearSampler, uv).w;

    // --- Signed CoC ---
    float signedCoc = linearDepth - g_DepthOfFieldParams.m_FocusDistance;
    float aperture = g_DepthOfFieldParams.m_Aperture;

    float cocNear = max(-signedCoc, 0.0) * aperture; // foreground blur
    float cocFar  = max(signedCoc, 0.0) * aperture;  // background blur

    cocNear = min(cocNear, g_DepthOfFieldParams.m_MaxCoC);
    cocFar  = min(cocFar,  g_DepthOfFieldParams.m_MaxCoC);

    // --- Sample bloom mips ---
    int mipNear = GetBloomMipLevel(cocNear);
    int mipFar  = GetBloomMipLevel(cocFar);

    float4 blurredNear = SampleBloomMip(mipNear, uv, linearSampler);
    float4 blurredFar  = SampleBloomMip(mipFar, uv, linearSampler);

    // --- Blend separately and combine ---
    float blendNear = saturate(cocNear / g_DepthOfFieldParams.m_MaxCoC);
    float blendFar  = saturate(cocFar  / g_DepthOfFieldParams.m_MaxCoC);

    float4 finalColor = lerp(color, blurredFar, blendFar);
    finalColor = lerp(finalColor, blurredNear, blendNear);

    g_DestinationTextureUav[threadID.xy] = finalColor;
}