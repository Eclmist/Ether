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
#include "common/fullscreenhelpers.hlsl"
#include "common/samplers.hlsl"

Texture2D albedo : register(t0);
Texture2D normal : register(t1);
Texture2D position : register(t2);

struct VS_OUTPUT
{
    float4 Position : SV_Position;
    float2 UV       : TEXCOORD0;
};

VS_OUTPUT VS_Main(uint ID : SV_VertexID)
{
    float2 pos;
    float2 uv;
    GetVertexFromID(ID, pos, uv);

    VS_OUTPUT o;
    o.Position = float4(pos, 1.0, 1.0);
    o.UV = uv;

    return o;
}

struct PointLight
{
    float3 m_Position;
    float3 m_Color;
    float m_Size;
    float m_Intensity;
 };

float3 ComputePointLight(float2 uv, PointLight light)
{
    float3 pos = position.Sample(g_PointSampler, uv).xyz;
    float3 col = albedo.Sample(g_PointSampler, uv).xyz;
    float3 norm = normal.Sample(g_PointSampler, uv).xyz;

    float3 dir = light.m_Position - pos;
    float d = length(dir);
    dir = normalize(dir);
    float3 L;

    float kc = 1.0;
    float Kl = 0.045;
    float Kq = 0.0075;
    float fAtt = light.m_Intensity / max((kc + Kl * d + Kq * d * d), 0.01);
    L = fAtt * light.m_Color;

    float ndotl = saturate(dot(norm, dir));
    float3 diffuse = col * ndotl;

    float3 r = reflect(dir, norm);
    float rdotv = saturate(dot(r, g_CommonConstants.EyeDirection.xyz));
    float n = 100.2;
    float3 specular = ndotl * pow(rdotv, n) * 0.5;

    return saturate(L * (diffuse + specular) * col);
}

float3 ComputeSkyLight(float2 uv)
{
    float skyConst = 0.5;
    float4 horizonColor = float4(255, 220, 213, 255) / 255.0 * skyConst;
    return horizonColor.xyz * 0.05 * 0;
}

float4 PS_Main(VS_OUTPUT IN) : SV_Target
{
    PointLight lights[7];

    float4 time = g_CommonConstants.Time * 1.0;

    lights[6].m_Position = float3(sin(time.z+2.9) * 2, -7.0, cos(time.z+ 5.2) * 2);
    lights[6].m_Color = float3(0.3, 0.1, 0.3);
    lights[6].m_Intensity = 6.0f;
    lights[6].m_Size = 10.0f;

    lights[1].m_Position = float3(10.0, sin(time.z) * 2.0, cos(time.y+10.3)* 6.0);
    lights[1].m_Color = float3(sin(time.w+0.12), 0.0, .3);
    lights[1].m_Intensity = 2.0f;
    lights[1].m_Size = 20.0f;

    lights[2].m_Position = float3(sin(time.y + 0.34) * 10.0, 9.0, 14.0);
    lights[2].m_Color = float3(0.3, 0.0, 1.0);
    lights[2].m_Intensity = 4.0f;
    lights[2].m_Size = 50.0f;

    lights[3].m_Position = float3(-10.0, 9.0, sin(time.y+ 1.4) * 4.0);
    lights[3].m_Color = float3(0.6, sin(time.y + 0.5), 0.1);
    lights[3].m_Intensity = 4.0f;
    lights[3].m_Size = 30.0f;

    lights[4].m_Position = float3(-2.0, -6.0, -14.0);
    lights[4].m_Color = float3(0.15, 0.83, 0.57);
    lights[4].m_Intensity = 8.0f;
    lights[4].m_Size = 40.0f;

    lights[5].m_Position = float3(4.5, 9.0, 16.0);
    lights[5].m_Color = float3(sin(time.y+1.6), 0.73, 0.57);
    lights[5].m_Intensity = 5.0f;
    lights[5].m_Size = 20.0f;

    lights[0].m_Position = float3(-60.5, 0.0, 0.0);
    lights[0].m_Color = float3(0.7, 0.78, 0.92);
    lights[0].m_Intensity = 30.0f;
    lights[0].m_Size = 100.0f;


    float3 finalColor = 0;

    for (int i = 0; i < 7; ++i)
    {
        finalColor += ComputePointLight(IN.UV, lights[i]) * 1.9;
    }

    finalColor = lerp(finalColor, float3(0.07, 0.06, 0.077), saturate(position.Sample(g_PointSampler, IN.UV).w / 100.0));

    return ComputeSkyLight(IN.UV).xyzz + finalColor.xyzz;

}
