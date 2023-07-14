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
#include "common/material.h"
#include "common/instanceparams.h"
#include "utils/fullscreenhelpers.hlsl"

struct VS_INPUT
{
    float3 Position     : POSITION;
    float3 Normal       : NORMAL;
    float3 Tangent      : TANGENT;
    float3 Bitangent    : BITANGENT;
    float2 TexCoord     : TEXCOORD;
};

struct VS_OUTPUT
{
    float4 Position     : SV_POSITION;
    float3 Normal       : NORMAL;
    float2 TexCoord     : TEXCOORD0;
    float4 WorldPos     : TEXCOORD1;
    float4 ClipPos      : TEXCOORD2;
    float3 Tangent      : TEXCOORD3;
    float3 Bitangent    : TEXCOORD4;
};

struct PS_OUTPUT
{
    float4 Output0      : SV_TARGET0;
    float4 Output1      : SV_TARGET1;
    float4 Output2      : SV_TARGET2;
    float4 Output3      : SV_TARGET3;
};

ConstantBuffer<GlobalConstants> g_GlobalConstants   : register(b0);
ConstantBuffer<InstanceParams> g_InstanceParams     : register(b1);
StructuredBuffer<Material> g_MaterialTable          : register(t0);

VS_OUTPUT VS_Main(VS_INPUT IN)
{
    VS_OUTPUT o;

    //float4x4 mv = mul(g_GlobalConstants.m_ViewMatrix, m_InstanceParams.m_ModelMatrix); (just leave model matrix as identity for now)
    float4x4 mvp = mul(g_GlobalConstants.m_ProjectionMatrix, g_GlobalConstants.m_ViewMatrix);
    float4x4 mvpPrev = mul(g_GlobalConstants.m_ProjectionMatrixPrev, g_GlobalConstants.m_ViewMatrixPrev);

    o.Position = mul(mvp, float4(IN.Position, 1.0f));
    o.ClipPos = mul(mvp, float4(IN.Position, 1.0f));
    o.WorldPos = /* mul(m_ModelMatrix, */ float4(IN.Position, 1.0f); //).xyz;
    o.Normal = IN.Normal;
    o.TexCoord = IN.TexCoord;
    o.Tangent = IN.Tangent;
    o.Bitangent = IN.Bitangent;

    return o;
}

PS_OUTPUT PS_Main(VS_OUTPUT IN)
{
    sampler linearSampler = SamplerDescriptorHeap[g_GlobalConstants.m_SamplerIndex_Linear_Wrap];
    Material material = g_MaterialTable[g_InstanceParams.m_MaterialIdx];

    const float2 resolution = g_GlobalConstants.m_ScreenResolution;
    float4x4 mvpPrev = mul(g_GlobalConstants.m_ProjectionMatrixPrev, g_GlobalConstants.m_ViewMatrixPrev);
    float2 texSpacePrev = ClipSpaceToTextureSpace(mul(mvpPrev, float4(IN.WorldPos.xyz, 1.0f)));
    float2 texSpaceCurr = ClipSpaceToTextureSpace(IN.ClipPos);
    float2 velocity = texSpaceCurr - texSpacePrev;

    float3 worldPos = IN.WorldPos.xyz;
    float4 albedo = material.m_BaseColor;
    float3 normal = IN.Normal.xyz;
    float roughness = 0.5;
    float metalness = 0;

    if (material.m_AlbedoTextureIndex != 0)
    {
        Texture2D<float4> albedoTex = ResourceDescriptorHeap[material.m_AlbedoTextureIndex];
        albedo *= albedoTex.Sample(linearSampler, IN.TexCoord);
    }

    if (material.m_NormalTextureIndex != 0)
    {
        Texture2D<float4> normalTex = ResourceDescriptorHeap[material.m_NormalTextureIndex];
        normal = normalTex.Sample(linearSampler, IN.TexCoord).xyz;
        normal = normal * 2.0 - 1.0;
        float3x3 TBN = float3x3(IN.Tangent, IN.Bitangent, IN.Normal.xyz);
        normal = normalize(mul(normal, TBN));
    }

    if (material.m_RoughnessTextureIndex != 0)
    {
        Texture2D<float4> roughnessTex = ResourceDescriptorHeap[material.m_RoughnessTextureIndex];
        roughness = sqrt(roughnessTex.Sample(linearSampler, IN.TexCoord).g);
    }

    if (material.m_MetalnessTextureIndex != 0)
    {
        Texture2D<float4> metalnessTex = ResourceDescriptorHeap[material.m_MetalnessTextureIndex];
        metalness = metalnessTex.Sample(linearSampler, IN.TexCoord).b;
    }

    // Don't support alpha yet
    if (InterleavedGradientNoise(IN.TexCoord.xy) > albedo.a)
        discard;

    PS_OUTPUT o;
    o.Output0 = float4(albedo.x,    albedo.y,   albedo.z,   1);
    o.Output1 = float4(worldPos.x,  worldPos.y, worldPos.z, roughness);
    o.Output2 = float4(normal.x,    normal.y,   normal.z,   metalness);
    o.Output3 = float4(velocity.x,  velocity.y, 0,          0);
    return o;
}