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

#ifndef __FORWARD_TRANSLUCENCY_PS__
#define __FORWARD_TRANSLUCENCY_PS__

#include "common/globalconstants.h"
#include "common/material.h"
#include "common/instanceparams.h"
#include "utils/brdf.hlsl"
#include "utils/shading.hlsl"
#include "utils/helpers.hlsl"

// RTCamp11-Only
ConstantBuffer<InstanceParams> g_InstanceParams : register(b1);
Texture2D<float4> g_SceneDepth : register(t1);

struct VS_INPUT
{
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float4 Color : COLOR;
    float2 TexCoord : TEXCOORD0;
    float3 PositionPrev : TEXCOORD1;
};

struct VS_OUTPUT
{
    float4 Position : SV_POSITION;
    float3 Normal : NORMAL;
    float4 Color : COLOR;
    float3 Tangent : TEXCOORD0;
    float2 TexCoord : TEXCOORD1;
    float4 ClipPos : TEXCOORD2;
    float4 ClipPosPrev : TEXCOORD3;
};

VS_OUTPUT VS_Main(VS_INPUT IN)
{
    VS_OUTPUT o;

    float4 worldPos = float4(IN.Position, 1.0f); // TODO: Implement model matrices here?
    float4 worldPosPrev = float4(IN.PositionPrev, 1.0f); // TODO: Implement model matrices here?

    sampler linearSampler = SamplerDescriptorHeap[g_GlobalConstants.m_SamplerIndex_Linear_Wrap];
    const Material material = g_MaterialTable[g_InstanceParams.m_MaterialIdx];

    if (material.m_BaseColorTextureIndex != 0)
    {
        Texture2D<float4> baseColor = ResourceDescriptorHeap[material.m_BaseColorTextureIndex];
        worldPos.y += baseColor.SampleLevel(linearSampler, IN.TexCoord * 0.05 + g_GlobalConstants.m_Time.w, 0).z * 0.05;
    }

    o.Position = mul(g_GlobalConstants.m_ViewProjectionMatrix, worldPos);
    o.Normal = IN.Normal;
    o.Tangent = IN.Tangent;
    o.TexCoord = IN.TexCoord;
    o.Color = IN.Color;
    o.ClipPos = mul(g_GlobalConstants.m_ViewProjectionMatrixNoJitter, worldPos);
    o.ClipPosPrev = mul(g_GlobalConstants.m_ViewProjectionMatrixPrevNoJitter, worldPosPrev);

    return o;
}

// Ref: http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html
uint ReverseBits32(uint bits)
{
    bits = (bits << 16) | (bits >> 16);
    bits = ((bits & 0x00ff00ff) << 8) | ((bits & 0xff00ff00) >> 8);
    bits = ((bits & 0x0f0f0f0f) << 4) | ((bits & 0xf0f0f0f0) >> 4);
    bits = ((bits & 0x33333333) << 2) | ((bits & 0xcccccccc) >> 2);
    bits = ((bits & 0x55555555) << 1) | ((bits & 0xaaaaaaaa) >> 1);
    return bits;
}

float VanDerCorputBase2(uint i)
{
    return ReverseBits32(i) * rcp(4294967296.0); // 2^-32
}

float2 Hammersley(uint i, uint sequenceLength)
{
    return float2(float(i) / float(sequenceLength), VanDerCorputBase2(i));
}

float3 SampleEnvironmentLighting(float3 wi, float mipLevel)
{
    sampler linearSampler = SamplerDescriptorHeap[g_GlobalConstants.m_SamplerIndex_Linear_Wrap];
    Texture2D<float4> hdriTexture = ResourceDescriptorHeap[g_GlobalConstants.m_HdriTextureIndex];
    const float exposure = g_GlobalConstants.m_SkyIntensity;

    const float2 hdriUv = SampleSphericalMap(wi);
    const float4 hdri = hdriTexture.SampleLevel(linearSampler, hdriUv, mipLevel);
    const float sunsetFactor = saturate(asin(dot(g_GlobalConstants.m_SunDirection.xyz, float3(0, 1, 0))));
    const float sunlightFactor = 1 - saturate(asin(dot(g_GlobalConstants.m_SunDirection.xyz, float3(0, -1, 0))));

    const float4 color = lerp(float4(0.5, 0.25, 0.25, 0), 1, sunsetFactor) * sunlightFactor;

    return (exposure * hdri * color).xyz;
}


#define NUM_SAMPLES 32

float3 BruteForcedIBL(ShadingSurface surface, float3 wo)
{
    sampler linearSampler = SamplerDescriptorHeap[g_GlobalConstants.m_SamplerIndex_Linear_Wrap];

    float3 ibl = 0;
    for (uint i = 0; i < NUM_SAMPLES; i++)
    {
        float2 Xi = Hammersley(i, NUM_SAMPLES);
        float3 H = normalize(ImportanceSampleGGX(Xi, wo, surface.m_Normal, surface.m_Roughness));
        float3 wi = normalize(2 * dot(wo, H) * H - wo);
        float NoV = saturate(dot(surface.m_Normal, wo));
        float NoL = saturate(dot(surface.m_Normal, wi));
        float NoH = saturate(dot(surface.m_Normal, H));
        float VoH = saturate(dot(wo, H));

        if (NoL > 0)
        {
            const float3 Li = SampleEnvironmentLighting(wi, 8);
            const float3 f = BRDF_UE4(wi, wo, surface.m_Normal, surface.m_BaseColor, surface.m_Roughness, surface.m_Metalness);

            ibl += Li * f * NoL;
        }
    }

    return ibl / NUM_SAMPLES;
}


struct PS_INPUT
{
    float4 ScreenPos : SV_POSITION;
    float3 Normal : NORMAL;
    float4 Color : COLOR;
    float3 Tangent : TEXCOORD0;
    float2 TexCoord : TEXCOORD1;
    float4 ClipPos : TEXCOORD2;
    float4 ClipPosPrev : TEXCOORD3;
};

float4 PS_Main(PS_INPUT IN) : SV_Target
{
    sampler linearSampler = SamplerDescriptorHeap[g_GlobalConstants.m_SamplerIndex_Linear_Wrap];
    const Material material = g_MaterialTable[g_InstanceParams.m_MaterialIdx];

    const float2 texSpaceCurr = ClipToTextureSpace(IN.ClipPos);
    const float2 texSpacePrev = ClipToTextureSpace(IN.ClipPosPrev);
    const float2 velocity = (texSpaceCurr - texSpacePrev);

    InterpolatedSurface interpolatedSurface;
    interpolatedSurface.m_VertexPosition = ClipToWorldSpace(IN.ClipPos);
    interpolatedSurface.m_Normal = IN.Normal;
    interpolatedSurface.m_Tangent = IN.Tangent;
    interpolatedSurface.m_Color = IN.Color;
    interpolatedSurface.m_TexCoord = IN.TexCoord;
    interpolatedSurface.m_TexCoord += float2(0.2, -1) * g_GlobalConstants.m_Time.z * 0.1;
    
    ShadingSurface surface = GetShadingSurfaceFromGeometry(interpolatedSurface, material, g_GlobalConstants.m_SamplerIndex_Linear_Wrap, -1);

    // Depth fade
    const float pixelDepth = LinearizeDepth(IN.ScreenPos.z);
    const float sceneDepth = LinearizeDepth(g_SceneDepth.Load(float3(IN.ScreenPos.xy, 0)).r);
    const float depthFade = sceneDepth - pixelDepth;

    surface.m_Roughness = 0.5;
    surface.m_BaseColor = lerp(surface.m_BaseColor, 1.0f, smoothstep(5.9, 0.89, depthFade));
    surface.m_Opacity = saturate(surface.m_Opacity * saturate(pow(depthFade / 5.0f, 1.3)));

    float3 Lo = 0;

    // Sun radiance
    const float lerpFactor = saturate(dot(g_GlobalConstants.m_SunDirection.xyz, float3(0, 1, 0)));
    float3 Li = lerp(0.0f, g_GlobalConstants.m_SunColor.xyz, lerpFactor);
    float3 wi = g_GlobalConstants.m_SunDirection.xyz;
    float3 wo = normalize(g_GlobalConstants.m_CameraPosition.xyz - surface.m_Position);
    const float3 f = BRDF_UE4(wi, wo, surface.m_Normal, surface.m_BaseColor, surface.m_Roughness, 0 /* TODO: Why is metalness input broken??*/);
    const float cosTheta = saturate(dot(wi, surface.m_Normal));
    Lo += f * Li * cosTheta * 0.75;

    // Sky radiance
    Lo += BruteForcedIBL(surface, wo) * 10.0f;


    return float4(Lo, surface.m_Opacity);
}

#endif // __FORWARD_TRANSLUCENCY_PS__