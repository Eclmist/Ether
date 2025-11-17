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

#ifndef __FORWARD_TRANSLUCENCY_PS__
#define __FORWARD_TRANSLUCENCY_PS__

#include "common/globalconstants.h"
#include "common/material.h"
#include "common/instanceparams.h"
#include "utils/brdf.hlsl"
#include "utils/shading.hlsl"
#include "utils/helpers.hlsl"
#include "utils/fullscreenhelpers.hlsl"

ConstantBuffer<InstanceParams> g_InstanceParams     : register(b1);
Texture2D<float4> g_SceneDepth                      : register(t1);

struct PS_INPUT
{
    float4 ScreenPos : SV_POSITION;
    float3 Normal : NORMAL;
    float4 Color : COLOR;
    float3 Tangent : TEXCOORD0;
    float2 TexCoord : TEXCOORD1;
};

float4 PS_Main(PS_INPUT IN) : SV_Target
{
    sampler linearSampler = SamplerDescriptorHeap[g_GlobalConstants.m_SamplerIndex_Linear_Wrap];
    const Material material = g_MaterialTable[g_InstanceParams.m_MaterialIdx];
    const float sceneDepth = g_SceneDepth.Load(float3(IN.ScreenPos.xy, 0)).r;

    InterpolatedSurface interpolatedSurface;
    interpolatedSurface.m_VertexPosition = ScreenToWorldSpace(IN.ScreenPos);
    interpolatedSurface.m_Normal = IN.Normal;
    interpolatedSurface.m_Tangent = IN.Tangent;
    interpolatedSurface.m_Color = IN.Color;
    interpolatedSurface.m_TexCoord = IN.TexCoord;
    
    ShadingSurface surface = GetShadingSurfaceFromGeometry(interpolatedSurface, material, g_GlobalConstants.m_SamplerIndex_Linear_Wrap, -1);
    float3 Lo = 0;

    // Sun radiance
    const float lerpFactor = saturate(dot(g_GlobalConstants.m_SunDirection.xyz, float3(0, 1, 0)));
    float3 Li = lerp(0.0f, g_GlobalConstants.m_SunColor.xyz, lerpFactor);
    float3 wi = g_GlobalConstants.m_SunDirection.xyz;
    float3 wo = normalize(g_GlobalConstants.m_CameraPosition.xyz - surface.m_Position);
    const float3 f = BRDF_UE4(wi, wo, surface.m_Normal, surface.m_BaseColor, surface.m_Roughness, surface.m_Metalness);
    const float cosTheta = saturate(dot(wi, surface.m_Normal));
    Lo += f * Li * cosTheta;

    return float4(Lo, surface.m_Opacity);
}

#endif // __FORWARD_TRANSLUCENCY_PS__