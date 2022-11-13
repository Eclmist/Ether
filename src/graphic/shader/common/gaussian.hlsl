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

#include "common/commonconstants.hlsl"
#include "common/fullscreenhelpers.hlsl"
#include "common/samplers.hlsl"

Texture2D SourceTexture : register(t0);

static const float GaussianKernel[5][5] = {
     {0.003, 0.013, 0.022, 0.013, 0.003},
     {0.013, 0.059, 0.097, 0.059, 0.013},
     {0.022, 0.097, 0.159, 0.097, 0.022},
     {0.013, 0.059, 0.097, 0.059, 0.013},
     {0.003, 0.013, 0.022, 0.013, 0.003}};
static const float BlurSize = 1;

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

float4 PS_Main_Horizontal(VS_OUTPUT IN) : SV_Target
{
    float4 col = 0;
    for (int i = -2; i <= 2; ++i)
    {
        for (int j = -2; j <= 2; ++j)
        {
            float2 uv = float2(IN.UV.x + i * BlurSize * ddx(IN.UV.x), IN.UV.y + j * BlurSize * ddy(IN.UV.y));
            col += SourceTexture.Sample(g_PointSampler, uv) * GaussianKernel[j + 2][i + 2];
        }
    }

    return col;
}
