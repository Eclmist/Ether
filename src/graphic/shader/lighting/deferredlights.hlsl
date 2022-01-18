/*
    This file is part of Ether, an open-source DirectX 12 renderer.

    Copyright (c) 2020-2021 Samuel Van Allen - All rights reserved.

    Ether is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

//#include "common/fullscreenhelpers.hlsl"

void GetVertexFromID(const uint vertexID, out float2 pos, out float2 uv)
{
    uint2 v = uint2(vertexID / 2, vertexID % 2);

    pos.x = v.x * 2.0 - 1.0;
    pos.y = v.y * 2.0 - 1.0;

    uv.x = v.x;
    uv.y = 1.0 - v.y;
}

struct VS_OUTPUT
{
    float4 Position : SV_Position;
    float2 UV       : TEXCOORD0;
};

Texture2D albedo : register(t0);
Texture2D normal : register(t1);
Texture2D position : register(t2);
SamplerState defaultSampler : register(s0);

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

float4 PS_Main(VS_OUTPUT IN) : SV_Target
{
    float3 lightDir = float3(1.2, 0.8, 0.3);
    //float3 eyeDir = IN.PositionES;
    float3 normalWS = normal.Sample(defaultSampler, IN.UV).xyz * 2.0 - 1.0;
    float4 col = albedo.Sample(defaultSampler, IN.UV);
    float ndotl = saturate(dot(normalWS, normalize(lightDir)));

    //float3 r = reflect(-lightDir, normal);
    //float rdotv = saturate(dot(r, eyeDir));
    //float n = 1000.2;

    float4 ambient = float4(0.4, 0.7, 0.4, 0.1) * 0.2;
    ambient.xyz += ((normalWS + 0.2) * 0.4);
    float4 diffuse = col * ndotl;
    //float4 specular = col * ndotl * pow(rdotv, n) * 0.5;

    return ambient + diffuse;
}
