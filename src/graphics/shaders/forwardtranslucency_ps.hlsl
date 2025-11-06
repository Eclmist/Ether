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

ConstantBuffer<InstanceParams> g_InstanceParams : register(b1);
StructuredBuffer<Material> g_MaterialTable : register(t0);

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

    const ShadingSurface surface = GetShadingSurfaceFromGeometry(interpolatedSurface, material, g_GlobalConstants.m_SamplerIndex_Linear_Wrap, -1);

    // Sun radiance
    const float lerpFactor = saturate(dot(g_GlobalConstants.m_SunDirection.xyz, float3(0, 1, 0)));
    float3 Li = lerp(0.0f, g_GlobalConstants.m_SunColor.xyz, lerpFactor);
    float3 wi = g_GlobalConstants.m_SunDirection.xyz;
    float3 wo = normalize(g_GlobalConstants.m_CameraPosition.xyz - surface.m_Position);
    const float3 f = BRDF_UE4(wi, wo, surface.m_Normal, surface.m_BaseColor, surface.m_Roughness, surface.m_Metalness);
    const float cosTheta = saturate(dot(wi, surface.m_Normal));
    float3 Lo = f * Li * cosTheta;

    return float4(Lo, surface.m_Opacity);
}

#endif // __FORWARD_TRANSLUCENCY_PS__