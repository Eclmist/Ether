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
#include "common/metadata.h"
#include "utils/brdf.hlsl"
#include "utils/shading.hlsl"
#include "utils/helpers.hlsl"
#include "utils/fullscreenhelpers.hlsl"

ConstantBuffer<InstanceParams> InstanceParams   : register(b1);
Texture2D<float2> SceneDepth                    : register(t1);

struct PS_INPUT
{
    float4 ScreenPos : SV_POSITION;
    float3 Normal    : NORMAL;
    float4 Color     : COLOR;
    float3 Tangent   : TEXCOORD0;
    float2 TexCoord  : TEXCOORD1;
};

struct PS_OUTPUT
{
    float4 Output    : SV_TARGET0;
#if ETH_TOOLMODE && WRITE_METADATA
    uint Metadata    : SV_TARGET1;
#endif
};


PS_OUTPUT PS_Main(PS_INPUT IN)
{
    sampler linearSampler = SamplerDescriptorHeap[GlobalConstants.m_SamplerIndex_Linear_Wrap];
    const Material material = MaterialTable[InstanceParams.m_MaterialIdx];
    const float sceneDepth = SceneDepth.Load(float3(IN.ScreenPos.xy, 0)).r;

    InterpolatedSurface interpolatedSurface;
    interpolatedSurface.m_VertexPosition = ScreenToWorldSpace(IN.ScreenPos);
    interpolatedSurface.m_Normal = IN.Normal;
    interpolatedSurface.m_Tangent = IN.Tangent;
    interpolatedSurface.m_Color = IN.Color;
    interpolatedSurface.m_TexCoord = IN.TexCoord;
    
    ShadingSurface surface = GetShadingSurfaceFromGeometry(interpolatedSurface, material, GlobalConstants.m_SamplerIndex_Linear_Wrap, -1);
    float3 Lo = 0;

    // Sun radiance
    const float lerpFactor = saturate(dot(GlobalConstants.m_SunDirection.xyz, float3(0, 1, 0)));
    float3 Li = lerp(0.0f, GlobalConstants.m_SunColor.xyz, lerpFactor);
    float3 wi = GlobalConstants.m_SunDirection.xyz;
    float3 wo = normalize(GlobalConstants.m_CameraPosition.xyz - surface.m_Position);
    const float3 f = BRDF_UE4(wi, wo, surface.m_Normal, surface.m_BaseColor, surface.m_Roughness, surface.m_Metalness);
    const float cosTheta = saturate(dot(wi, surface.m_Normal));
    Lo += f * Li * cosTheta;

    PS_OUTPUT o;
    o.Output = float4(Lo, surface.m_Opacity);

#if ETH_TOOLMODE && WRITE_METADATA
    if (GlobalConstants.m_TranslucentPickingEnabled)
    {
        Metadata metadata;
        metadata.m_IsValid = true;
        metadata.m_EntityID = InstanceParams.m_EntityID;
        o.Metadata = PackMetadata(metadata);
    }
#endif

    return o;

}

#endif // __FORWARD_TRANSLUCENCY_PS__