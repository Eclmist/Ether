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

Texture2D SourceTexture : register(t0);
Texture2D BloomTexture1 : register(t1);
Texture2D BloomTexture2 : register(t2);
Texture2D BloomTexture3 : register(t3);
Texture2D BloomTexture4 : register(t4);
Texture2D BloomTexture5 : register(t5);
Texture2D BloomTexture6 : register(t6);

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

float4 PS_HighPass(VS_OUTPUT IN) : SV_Target
{
    float4 col = SourceTexture.Sample(g_BilinearSampler, IN.UV);

    float lum = col.r * 0.3 + col.g * 0.59 + col.b * 0.11;
    if (lum >= 1.0)
        return col;
    
    return 0;
}

float4 PS_Composite(VS_OUTPUT IN) : SV_Target
{
    float4 col = SourceTexture.Sample(g_BilinearSampler, IN.UV);
    float4 bloom = BloomTexture1.Sample(g_BilinearSampler, IN.UV) * 0.1;
    bloom += BloomTexture2.Sample(g_BilinearSampler, IN.UV) * 0.2;
    bloom += BloomTexture3.Sample(g_BilinearSampler, IN.UV) * 0.3;
    bloom += BloomTexture4.Sample(g_BilinearSampler, IN.UV) * 0.4;
    bloom += BloomTexture5.Sample(g_BilinearSampler, IN.UV) * 0.5;
    bloom += BloomTexture6.Sample(g_BilinearSampler, IN.UV) * 0.6;

    float4 flare1 = BloomTexture6.Sample(g_BilinearSampler, (1-IN.UV) * 0.5 + 0.25) * float4(0.6, 0.7, 0.9, 1.0);
    float4 flare2 = BloomTexture6.Sample(g_BilinearSampler, (1-IN.UV) * 0.8 + 0.1) * float4(0.8, 0.3, 0.9, 1.0);
    float4 flare3 = BloomTexture6.Sample(g_BilinearSampler, (1-IN.UV) * 0.3 + 0.35) * float4(0.2, 0.7, 0.9, 1.0);
    float4 flare4 = BloomTexture6.Sample(g_BilinearSampler, (1-IN.UV) * 1.0 + 0.0) * float4(1.0, 0.7, 0.9, 1.0);
    float4 flare5 = BloomTexture6.Sample(g_BilinearSampler, (1-IN.UV) * 0.1 + 0.45) * float4(0.6, 0.3, 0.9, 1.0);
    float4 flare6 = BloomTexture6.Sample(g_BilinearSampler, (1-IN.UV) * 2.0 - 1.0) * float4(0.6, 0.3, 0.9, 1.0);
    
    return col + bloom * 0.01 + (flare1 + flare2 + flare3 + flare4 + flare5) * 0.3;
}