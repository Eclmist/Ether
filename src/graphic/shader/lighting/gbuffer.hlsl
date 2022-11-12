/*
    This file is part of Ether, an open-source DirectX 12 renderer.

    Copyright (c) 2020-2022 Samuel Huang - All rights reserved.

    Ether is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "common/commonconstants.hlsl"
#include "common/samplers.hlsl"

struct InstanceParams
{
    struct MaterialParams
    {
        float4 m_BaseColor;
        float4 m_SpecularColor;
        float m_Roughness;
        float m_Metalness;

        uint m_AlbedoTextureIndex;
        uint m_SpecularTextureIndex;
    };

    MaterialParams m_MaterialParams;

    float4x4 m_ModelMatrix;
    float4x4 m_NormalMatrix;

#ifdef ETH_TOOLMODE
    uint4 m_PickerColor;
#endif
};

ConstantBuffer<InstanceParams> m_InstanceParams : register(b1);

struct VS_INPUT
{
    float3 Position : POSITION;
    float3 Normal   : NORMAL;
    float4 Tangent  : TANGENT0;
    float2 TexCoord : TEXCOORD0;
};

struct VS_OUTPUT
{
    float4 Position     : SV_Position;
    float3 NormalWS     : NORMAL;
    float3 PositionWS   : TEXCOORD0;
    float2 UV           : TEXCOORD1;
};

struct PS_OUTPUT
{
    float4 Albedo       : SV_Target0;
    float4 Specular     : SV_Target1;
    float4 Normal       : SV_Target2;
    float4 Position     : SV_Target3;

#ifdef ETH_TOOLMODE
    float4 Picker       : SV_Target4;
#endif
};

VS_OUTPUT VS_Main(VS_INPUT IN, uint ID: SV_InstanceID)
{
    VS_OUTPUT o;

    float3 pos = IN.Position;

    float4x4 mv = mul(g_CommonConstants.ViewMatrix, m_InstanceParams.m_ModelMatrix);
    float4x4 mvp = mul(g_CommonConstants.ProjectionMatrix, mv);

    o.Position = mul(mvp, float4(pos, 1.0));
    o.PositionWS = mul(m_InstanceParams.m_ModelMatrix, float4(pos, 1.0)).xyz;
    o.NormalWS = mul(m_InstanceParams.m_NormalMatrix, float4(IN.Normal, 1.0)).xyz;
    o.UV = IN.TexCoord;

    return o;
}

PS_OUTPUT PS_Main(VS_OUTPUT IN)
{
    Texture2D m_AlbedoTex = ResourceDescriptorHeap[m_InstanceParams.m_MaterialParams.m_AlbedoTextureIndex];
    Texture2D m_SpecularTex = ResourceDescriptorHeap[m_InstanceParams.m_MaterialParams.m_SpecularTextureIndex];

    float4 col = m_AlbedoTex.Sample(g_PointSampler, float2(IN.UV.x, -IN.UV.y)); // TODO: why is uv.y flipped?
    float4 spec = m_SpecularTex.Sample(g_PointSampler, float2(IN.UV.x, -IN.UV.y));
    float3 normal = normalize(IN.NormalWS);
    float3 positionWS = IN.PositionWS;

    PS_OUTPUT output;
    output.Albedo.xyz = col.xyz * m_InstanceParams.m_MaterialParams.m_BaseColor.xyz;
    output.Albedo.w = 0.75;// max(m_InstanceParams.m_MaterialParams.m_Roughness, 0.02);
    output.Specular.xyz = 1.0 - spec.xyz * m_InstanceParams.m_MaterialParams.m_SpecularColor.xyz;
    output.Specular.w = m_InstanceParams.m_MaterialParams.m_Metalness;
    output.Normal = normal.xyzz;
    output.Position.xyz = positionWS.xyz;
    output.Position.w = mul(g_CommonConstants.ViewMatrix, float4(positionWS, 1.0)).z;

#ifdef ETH_TOOLMODE
    output.Picker = m_InstanceParams.m_PickerColor / 255.0;
#endif

    return output;
}