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

#ifndef __SHADING_HLSL__
#define __SHADING_HLSL__

#include "common/vertexcommon.h"
#include "common/material.h"
#include "utils/fullscreenhelpers.hlsl"
#include "utils/encoding.hlsl"

#define EMISSION_SCALE 10000.0f

struct InterpolatedSurface
{
    float3 m_VertexPosition;
    float3 m_Normal;
    float3 m_Tangent;
    float2 m_TexCoord;
    float4 m_Color;
};

struct ShadingSurface
{
    float3 m_Position;
    float3 m_Normal;
    float3 m_BaseColor;
    float3 m_Emission;
    float m_Roughness;
    float m_Metalness;
    float m_Opacity;
    float2 m_Velocity;

    int m_MaterialID;
};

ShadingSurface GetShadingSurfaceFromGBuffers(
    float2 screenCoord,
    Texture2D gbufferTexA,
    Texture2D gBufferTexB,
    Texture2D gBufferTexC,
    Texture2D<float2> sceneDepth)
{
    const float4 gbufferA = gbufferTexA.Load(int3(screenCoord, 0));
    const float4 gbufferB = gBufferTexB.Load(int3(screenCoord, 0));
    const float4 gbufferC = gBufferTexC.Load(int3(screenCoord, 0));
    const float depth = sceneDepth.Load(int3(screenCoord, 0)).r;

    ShadingSurface surface;
    surface.m_Position = ScreenToWorldSpace(screenCoord, depth);
    surface.m_Normal = DecodeNormals(gbufferB.xy);
    surface.m_BaseColor = gbufferA.rgb;
    surface.m_Emission = gbufferC.rgb;
    surface.m_Roughness = DecodeFP16(gbufferC.w).x;
    surface.m_Metalness = DecodeFP16(gbufferC.w).y;
    surface.m_Velocity = gbufferB.zw;
    surface.m_MaterialID = floor(gbufferA.w * 255.0f);

    // Opacity information is lost after gbuffer discards alpha masked pixels.
    // For actually translucent pixels, they're not drawn in the gbuffer at all.
    surface.m_Opacity = 1.0f;

    return surface;
}

ShadingSurface GetShadingSurfaceFromGeometry(InterpolatedSurface interpolatedSurface, Material material, uint samplerIndex, float mipLevel)
{
    sampler linearSampler = SamplerDescriptorHeap[samplerIndex];
    float3 baseColor = material.m_BaseColor * interpolatedSurface.m_Color.rgb;
    float3 emission = material.m_EmissiveColor.rgb * EMISSION_SCALE;
    float3 normal = interpolatedSurface.m_Normal;
    float roughness = material.m_Roughness;
    float metalness = material.m_Metalness;
    float opacity = material.m_Opacity * interpolatedSurface.m_Color.a;

    if (material.m_BaseColorTextureIndex != 0)
    {
        Texture2D<float4> baseColorTex = ResourceDescriptorHeap[material.m_BaseColorTextureIndex];
        float4 baseColorSample;
        if (mipLevel != -1)
            baseColorSample = baseColorTex.SampleLevel(linearSampler, interpolatedSurface.m_TexCoord, mipLevel);
        else
            baseColorSample = baseColorTex.Sample(linearSampler, interpolatedSurface.m_TexCoord);

        baseColor *= baseColorSample.rgb;
        opacity *= baseColorSample.a;
    }
    if (material.m_NormalTextureIndex != 0)
    {
        Texture2D<float4> normalTex = ResourceDescriptorHeap[material.m_NormalTextureIndex];

        if (mipLevel != -1)
            normal = normalTex.SampleLevel(linearSampler, interpolatedSurface.m_TexCoord, mipLevel).xyz;
        else
            normal = normalTex.Sample(linearSampler, interpolatedSurface.m_TexCoord).xyz;

        normal = normal * 2.0 - 1.0;
        float3 bitangent = cross(interpolatedSurface.m_Tangent, interpolatedSurface.m_Normal);
        float3x3 TBN = float3x3(interpolatedSurface.m_Tangent, bitangent, interpolatedSurface.m_Normal.xyz);
        normal = normalize(mul(normal, TBN));
    }
    if (material.m_RoughnessTextureIndex != 0)
    {
        Texture2D<float4> roughnessTex = ResourceDescriptorHeap[material.m_RoughnessTextureIndex];

        if (mipLevel != -1)
            roughness = roughnessTex.SampleLevel(linearSampler, interpolatedSurface.m_TexCoord, mipLevel).g;
        else
            roughness = roughnessTex.Sample(linearSampler, interpolatedSurface.m_TexCoord).g;
    }
    if (material.m_MetalnessTextureIndex != 0)
    {
        Texture2D<float4> metalnessTex = ResourceDescriptorHeap[material.m_MetalnessTextureIndex];

        if (mipLevel != -1)
            metalness = metalnessTex.SampleLevel(linearSampler, interpolatedSurface.m_TexCoord, mipLevel).b;
        else
            metalness = metalnessTex.Sample(linearSampler, interpolatedSurface.m_TexCoord).b;
    }
    if (material.m_EmissiveTextureIndex != 0)
    {
        Texture2D<float4> emissiveTex = ResourceDescriptorHeap[material.m_EmissiveTextureIndex];

        if (mipLevel != -1)
            emission *= emissiveTex.SampleLevel(linearSampler, interpolatedSurface.m_TexCoord, mipLevel).rgb;
        else
            emission *= emissiveTex.Sample(linearSampler, interpolatedSurface.m_TexCoord).rgb;
    }

    ShadingSurface shadingSurface;
    shadingSurface.m_Position = interpolatedSurface.m_VertexPosition;
    shadingSurface.m_Normal = normalize(normal);
    shadingSurface.m_BaseColor = saturate(baseColor);
    shadingSurface.m_Roughness = clamp(roughness, 0.01f, 1.0f);
    shadingSurface.m_Metalness = saturate(metalness);
    shadingSurface.m_Emission = emission;
    shadingSurface.m_Opacity = saturate(opacity);
    shadingSurface.m_Velocity = 0.0f;
    shadingSurface.m_MaterialID = material.m_MaterialId;

    return shadingSurface;
}

ShadingSurface GetShadingSurfaceFromHit(MeshVertex hitSurface, Material material, uint samplerIndex, float mipLevel)
{
    InterpolatedSurface geometricSurface;
    geometricSurface.m_VertexPosition = mul(ObjectToWorld3x4(), float4(hitSurface.m_Position, 1.0f));
    geometricSurface.m_Normal = mul(ObjectToWorld3x4(), float4(hitSurface.m_Normal, 0.0f));
    geometricSurface.m_Tangent = hitSurface.m_Tangent;
    geometricSurface.m_Color = hitSurface.m_Color;
    geometricSurface.m_TexCoord = hitSurface.m_TexCoord;

    ShadingSurface shadingSurface = GetShadingSurfaceFromGeometry(geometricSurface, material, samplerIndex, mipLevel);
    return shadingSurface;
}

#endif // __SHADING_HLSL__
