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

#include "common/vertexcommon.h"
#include "common/material.h"

#define EMISSION_SCALE 10000.0f

struct GeometricSurface
{
    float3 m_Position;
    float3 m_Normal;
    float3 m_Tangent;
    float2 m_TexCoord;
};

struct ShadingSurface
{
    float3 m_Position;
    float3 m_Normal;
    float3 m_Albedo;
    float3 m_Emission;
    float m_Roughness;
    float m_Metalness;
    float m_Opacity;
    float2 m_Velocity;

    int m_MaterialID;
};

ShadingSurface GetShadingSurfaceFromGBuffers(
    float2 screenCoord,
    Texture2D gbufferA,
    Texture2D gbufferB,
    Texture2D gbufferC,
    Texture2D gbufferD
)
{
    const float4 gbuffer0 = gbufferA.Load(int3(screenCoord, 0));
    const float4 gbuffer1 = gbufferB.Load(int3(screenCoord, 0));
    const float4 gbuffer2 = gbufferC.Load(int3(screenCoord, 0));
    const float4 gbuffer3 = gbufferD.Load(int3(screenCoord, 0));

    ShadingSurface surface;
    surface.m_Position = gbuffer1.xyz;
    surface.m_Normal = DecodeNormals(gbuffer2.xy);
    surface.m_Albedo = gbuffer0.rgb;
    surface.m_Emission = gbuffer3.rgb;
    surface.m_Roughness = gbuffer1.w;
    surface.m_Metalness = gbuffer0.w;
    surface.m_Velocity = gbuffer2.zw;
    surface.m_MaterialID = gbuffer3.w;
    return surface;
}

ShadingSurface GetShadingSurfaceFromGeometry(GeometricSurface geometricSurface, Material material, uint samplerIndex, float mipLevel)
{
    sampler linearSampler = SamplerDescriptorHeap[samplerIndex];

    float4 albedo = material.m_BaseColor;
    float3 emission = material.m_EmissiveColor.rgb * EMISSION_SCALE;
    float3 normal = geometricSurface.m_Normal;
    float roughness = 0.5f;
    float metalness = 0.0f;
    float opacity = 1.0f;

    if (material.m_AlbedoTextureIndex != 0)
    {
        Texture2D<float4> albedoTex = ResourceDescriptorHeap[material.m_AlbedoTextureIndex];

        if (mipLevel != -1)
            albedo *= albedoTex.SampleLevel(linearSampler, geometricSurface.m_TexCoord, mipLevel);
        else
            albedo *= albedoTex.Sample(linearSampler, geometricSurface.m_TexCoord);
    }
    if (material.m_NormalTextureIndex != 0)
    {
        Texture2D<float4> normalTex = ResourceDescriptorHeap[material.m_NormalTextureIndex];

        if (mipLevel != -1)
            normal = normalTex.SampleLevel(linearSampler, geometricSurface.m_TexCoord, mipLevel).xyz;
        else
            normal = normalTex.Sample(linearSampler, geometricSurface.m_TexCoord).xyz;

        normal = normal * 2.0 - 1.0;
        float3 bitangent = cross(geometricSurface.m_Tangent, geometricSurface.m_Normal);
        float3x3 TBN = float3x3(geometricSurface.m_Tangent, bitangent, geometricSurface.m_Normal.xyz);
        normal = normalize(mul(normal, TBN));
    }
    if (material.m_RoughnessTextureIndex != 0)
    {
        Texture2D<float4> roughnessTex = ResourceDescriptorHeap[material.m_RoughnessTextureIndex];

        if (mipLevel != -1)
            roughness = roughnessTex.SampleLevel(linearSampler, geometricSurface.m_TexCoord, mipLevel).g;
        else
            roughness = roughnessTex.Sample(linearSampler, geometricSurface.m_TexCoord).g;
    }
    if (material.m_MetalnessTextureIndex != 0)
    {
        Texture2D<float4> metalnessTex = ResourceDescriptorHeap[material.m_MetalnessTextureIndex];

        if (mipLevel != -1)
            metalness = metalnessTex.SampleLevel(linearSampler, geometricSurface.m_TexCoord, mipLevel).b;
        else
            metalness = metalnessTex.Sample(linearSampler, geometricSurface.m_TexCoord).b;
    }
    if (material.m_EmissiveTextureIndex != 0)
    {
        Texture2D<float4> emissiveTex = ResourceDescriptorHeap[material.m_EmissiveTextureIndex];

        if (mipLevel != -1)
            emission *= emissiveTex.SampleLevel(linearSampler, geometricSurface.m_TexCoord, mipLevel).rgb;
        else
            emission *= emissiveTex.Sample(linearSampler, geometricSurface.m_TexCoord).rgb;
    }

    ShadingSurface shadingSurface;
    shadingSurface.m_Position = geometricSurface.m_Position;
    shadingSurface.m_Normal = normal;
    shadingSurface.m_Albedo = albedo.rgb;
    shadingSurface.m_Roughness = roughness;
    shadingSurface.m_Metalness = metalness;
    shadingSurface.m_Emission = emission;
    shadingSurface.m_Opacity = albedo.a;
    shadingSurface.m_Velocity = 0.0f;
    shadingSurface.m_MaterialID = material.m_MaterialId;

    return shadingSurface;
}

ShadingSurface GetShadingSurfaceFromHit(MeshVertex hitSurface, Material material, uint samplerIndex, float mipLevel)
{
    GeometricSurface geometricSurface;
    geometricSurface.m_Position = hitSurface.m_Position;
    geometricSurface.m_Normal = hitSurface.m_Normal;
    geometricSurface.m_Tangent = hitSurface.m_Tangent;
    geometricSurface.m_TexCoord = hitSurface.m_TexCoord;

    ShadingSurface shadingSurface = GetShadingSurfaceFromGeometry(geometricSurface, material, samplerIndex, mipLevel);
    shadingSurface.m_Normal = dot(-WorldRayDirection(), shadingSurface.m_Normal) < 0 ? -shadingSurface.m_Normal : shadingSurface.m_Normal; // Fix lightleakage
    return shadingSurface;
}


