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
#include "utils/encoding.hlsl"

struct VS_INPUT
{
    float3 Position     : POSITION;
    float3 Normal       : NORMAL;
    float3 Tangent      : TANGENT;
    float2 TexCoord     : TEXCOORD;
};

struct VS_OUTPUT
{
    float4 Position     : SV_POSITION;
    float3 Normal       : NORMAL;
    float2 TexCoord     : TEXCOORD0;
    float3 Tangent      : TEXCOORD1;
};

struct PS_OUTPUT
{
    float4 Output0      : SV_TARGET0;
    float4 Output1      : SV_TARGET1;
    float4 Output2      : SV_TARGET2;
};

ConstantBuffer<GlobalConstants> g_GlobalConstants   : register(b0);
ConstantBuffer<InstanceParams> g_InstanceParams     : register(b1);
StructuredBuffer<Material> g_MaterialTable          : register(t0);

VS_OUTPUT VS_Main(VS_INPUT IN)
{
    VS_OUTPUT o;

    o.Position = mul(g_GlobalConstants.m_ViewProjectionMatrix, float4(IN.Position, 1.0f));
    o.Normal = IN.Normal;
    o.TexCoord = IN.TexCoord;
    o.Tangent = IN.Tangent;

    return o;
}

PS_OUTPUT PS_Main(VS_OUTPUT IN)
{
    sampler linearSampler = SamplerDescriptorHeap[g_GlobalConstants.m_SamplerIndex_Linear_Wrap];
    Material material = g_MaterialTable[g_InstanceParams.m_MaterialIdx];

    float4 clipPos = ScreenToClipSpace(IN.Position, g_GlobalConstants.m_ScreenResolution);
    float4 worldPos = mul(g_GlobalConstants.m_ViewProjectionMatrixInv, clipPos);
    float4 clipPosPrev = mul(g_GlobalConstants.m_ViewProjectionMatrixPrev, worldPos);
    float2 texSpacePrev = ClipToTextureSpace(clipPosPrev);
    float2 texSpaceCurr = ScreenToTextureSpace(IN.Position, g_GlobalConstants.m_ScreenResolution);

    float4 albedo = material.m_BaseColor;
    float3 normal = IN.Normal.xyz;
    float roughness = 0.5;
    float metalness = 0;
    float2 velocity = texSpaceCurr - texSpacePrev;

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
        normal = normalize(normal);
        float3 bitangent = normalize(cross(IN.Normal, IN.Tangent));
        float3x3 TBN = float3x3(IN.Tangent, bitangent, IN.Normal);
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

    float2 octNormals = EncodeNormals(normal);

    PS_OUTPUT o;
    o.Output0 = float4(albedo.x,     albedo.y,      albedo.z,   IN.Position.w);
    o.Output1 = float4(worldPos.x,   worldPos.y,    worldPos.z, roughness);
    o.Output2 = float4(octNormals.x, octNormals.y,  velocity.x, velocity.y);
    return o;
}