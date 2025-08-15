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

#include "common/material.h"

struct ShadingSurface
{
    float3 m_Position;
    float3 m_Normal;
    float3 m_Albedo;
    float3 m_Emission;
    float m_Roughness;
    float m_Metalness;
    float2 m_Velocity;
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
    return surface;
}

ShadingSurface GetShadingSurfaceFromHit(MeshVertex hitSurface, Material material, uint samplerIndex, float mipLevel)
{
    sampler linearSampler = SamplerDescriptorHeap[samplerIndex];

    float3 albedo = material.m_BaseColor.rgb;
    float3 emission = material.m_EmissiveColor.rgb;
    float3 normal = hitSurface.m_Normal;
    float roughness = 1;
    float metalness = 0;

    if (material.m_AlbedoTextureIndex != 0)
    {
        Texture2D<float4> albedoTex = ResourceDescriptorHeap[material.m_AlbedoTextureIndex];
        albedo *= albedoTex.SampleLevel(linearSampler, hitSurface.m_TexCoord, mipLevel).rgb;
    }
    if (material.m_NormalTextureIndex != 0)
    {
        Texture2D<float4> normalTex = ResourceDescriptorHeap[material.m_NormalTextureIndex];
        normal = normalTex.SampleLevel(linearSampler, hitSurface.m_TexCoord, mipLevel).xyz;
        normal = normal * 2.0 - 1.0;
        float3 bitangent = cross(hitSurface.m_Tangent, hitSurface.m_Normal);
        float3x3 TBN = float3x3(hitSurface.m_Tangent, bitangent, hitSurface.m_Normal.xyz);
        normal = normalize(mul(normal, TBN));
    }
    if (material.m_RoughnessTextureIndex != 0)
    {
        Texture2D<float4> roughnessTex = ResourceDescriptorHeap[material.m_RoughnessTextureIndex];
        roughness = 1 - roughnessTex.SampleLevel(linearSampler, hitSurface.m_TexCoord, mipLevel).g;
    }
    if (material.m_MetalnessTextureIndex != 0)
    {
        Texture2D<float4> metalnessTex = ResourceDescriptorHeap[material.m_MetalnessTextureIndex];
        metalness = metalnessTex.SampleLevel(linearSampler, hitSurface.m_TexCoord, mipLevel).b;
    }
    if (material.m_EmissiveTextureIndex != 0)
    {
        Texture2D<float4> emissiveTex = ResourceDescriptorHeap[material.m_EmissiveTextureIndex];
        emission *= emissiveTex.SampleLevel(linearSampler, hitSurface.m_TexCoord, mipLevel).rgb;
    }

    ShadingSurface shadingSurface;
    shadingSurface.m_Position = hitSurface.m_Position;
    shadingSurface.m_Normal = normal;
    shadingSurface.m_Albedo = albedo;
    shadingSurface.m_Roughness = roughness;
    shadingSurface.m_Metalness = metalness;
    shadingSurface.m_Emission = emission;

    return shadingSurface;
}


