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
#include "utils/noise.hlsl"
#include "utils/shading.hlsl"

struct VS_INPUT
{
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float2 TexCoord : TEXCOORD;
    float4 PrevPosition : COLOR;
};

struct VS_OUTPUT
{
    float4 Position : SV_POSITION;
    float3 Normal : NORMAL;
    float2 TexCoord : TEXCOORD0;
    float3 Tangent : TEXCOORD1;
    float3 WorldPos : TEXCOORD2;
    float4 PrevClipPos : TEXCOORD3;
};

struct PS_OUTPUT
{
    float4 Output0 : SV_TARGET0;
    float4 Output1 : SV_TARGET1;
    float4 Output2 : SV_TARGET2;
    float3 Output3 : SV_TARGET3;
};

ConstantBuffer<GlobalConstants> g_GlobalConstants : register(b0);
ConstantBuffer<InstanceParams> g_InstanceParams : register(b1);
StructuredBuffer<Material> g_MaterialTable : register(t0);

float4x4 RemoveJitter(float4x4 jitteredProjMatrix, float2 jitter)
{
    float4x4 inverseJitterMatrix = float4x4(
        1, 0, -jitter.x, 0,
        0, 1, -jitter.y, 0,
        0, 0, 1, 0,
        0, 0, 0, 1);
    float4x4 originalProjMatrix = mul(inverseJitterMatrix, jitteredProjMatrix);
    return originalProjMatrix;
}

VS_OUTPUT VS_Main(VS_INPUT IN)
{
    VS_OUTPUT o;

    o.Position = mul(g_GlobalConstants.m_ViewProjectionMatrix, float4(IN.Position, 1.0f));
    o.Normal = IN.Normal;
    o.TexCoord = IN.TexCoord;
    o.Tangent = IN.Tangent;
    o.WorldPos = IN.Position; // TODO: Add model matrix
    o.PrevClipPos = mul(RemoveJitter(g_GlobalConstants.m_ViewProjectionMatrixPrev, g_GlobalConstants.m_CameraJitterPrev), float4(IN.PrevPosition.xyz, 1.0f));

    return o;
}

PS_OUTPUT PS_Main(VS_OUTPUT IN)
{
    sampler linearSampler = SamplerDescriptorHeap[g_GlobalConstants.m_SamplerIndex_Linear_Wrap];
    const Material material = g_MaterialTable[g_InstanceParams.m_MaterialIdx];

    const float4 clipPosCurr = mul(RemoveJitter(g_GlobalConstants.m_ViewProjectionMatrix, g_GlobalConstants.m_CameraJitter), float4(IN.WorldPos, 1.0f));
    const float2 texSpacePrev = ClipToTextureSpace(IN.PrevClipPos);
    const float2 texSpaceCurr = ClipToTextureSpace(clipPosCurr);
    const float2 velocity = (texSpaceCurr - texSpacePrev);

    GeometricSurface geometricSurface;
    geometricSurface.m_Position = IN.WorldPos;
    geometricSurface.m_Normal = IN.Normal;
    geometricSurface.m_Tangent = IN.Tangent;
    geometricSurface.m_TexCoord = IN.TexCoord;

    const ShadingSurface shadingSurface = GetShadingSurfaceFromGeometry(geometricSurface, material, g_GlobalConstants.m_SamplerIndex_Linear_Wrap, -1);

    const float3 worldPos = shadingSurface.m_Position;
    const float2 octNormals = EncodeNormals(shadingSurface.m_Normal);
    const float3 albedo = shadingSurface.m_Albedo;
    const float3 emissive = shadingSurface.m_Emission;
    const float roughness = shadingSurface.m_Roughness;
    const float metalness = shadingSurface.m_Metalness;
    const float opacity = shadingSurface.m_Opacity;
    
    // Dither non-opaque surfaces in gbuffer
    if (InterleavedGradientNoise(IN.TexCoord) > shadingSurface.m_Opacity)
        discard;

    PS_OUTPUT o;
    o.Output0 = float4(albedo.x, albedo.y, albedo.z, metalness);
    o.Output1 = float4(worldPos.x, worldPos.y, worldPos.z, roughness);
    o.Output2 = float4(octNormals.x, octNormals.y, velocity.x, velocity.y);
    o.Output3 = float3(emissive.x, emissive.y, emissive.z);
    return o;
}