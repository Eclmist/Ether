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

#ifndef __GBUFFER_PS_HLSL__
#define __GBUFFER_PS_HLSL__

#include "common/globalconstants.h"
#include "common/material.h"
#include "common/instanceparams.h"
#include "utils/shading.hlsl"

struct PS_INPUT
{
    float4 ScreenPos        : SV_POSITION;
    float3 Normal           : NORMAL;
    float4 Color            : COLOR;
    float3 Tangent          : TEXCOORD0;
    float2 TexCoord         : TEXCOORD1;
    float4 ClipPos          : TEXCOORD2;
    float4 ClipPosPrev      : TEXCOORD3;
};

struct PS_OUTPUT
{
    float4 Output0 : SV_TARGET0;
    float4 Output1 : SV_TARGET1;
    float4 Output2 : SV_TARGET2;
};

ConstantBuffer<InstanceParams> g_InstanceParams     : register(b1);

void DiscardAlphaMaskedPixels(const ShadingSurface surface)
{
    if (surface.m_Opacity < 0.5f)
        discard;
}

PS_OUTPUT PS_Main(PS_INPUT IN)
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

    const ShadingSurface shadingSurface = GetShadingSurfaceFromGeometry(interpolatedSurface, material, g_GlobalConstants.m_SamplerIndex_Linear_Wrap, -1);

    const float3 worldPos = shadingSurface.m_Position;
    const float3 normal = shadingSurface.m_Normal;
    const float3 baseColor = shadingSurface.m_BaseColor;
    const float3 emissive = shadingSurface.m_Emission * 2;
    const float roughness = shadingSurface.m_Roughness;
    const float metalness = shadingSurface.m_Metalness;
    const float opacity = shadingSurface.m_Opacity;

    DiscardAlphaMaskedPixels(shadingSurface);

    PS_OUTPUT o;
    o.Output0 = float4(baseColor.x, baseColor.y, baseColor.z, (g_InstanceParams.m_MaterialIdx / 255.0f));
    o.Output1 = float4(EncodeNormals(normal), velocity.x, velocity.y);
    o.Output2 = float4(emissive.x, emissive.y, emissive.z, EncodeFP16(roughness, metalness));
    return o;
}

#endif // __GBUFFER_PS_HLSL__
