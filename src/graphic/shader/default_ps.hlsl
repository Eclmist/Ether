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

struct GlobalConstants
{
    float4 Time;
};

ConstantBuffer<GlobalConstants> CB_GlobalConstants : register(b0);

struct PS_INPUT
{
    float4 Position : SV_Position;
    float4 Color    : COLOR;
};

float distLine(float2 p, float2 a, float2 b) {
    float2 ap = p - a;
    float2 ab = b - a;
    float aDotB = clamp(dot(ap, ab) / dot(ab, ab), 0.0, 1.0);
    return length(ap - ab * aDotB);
}

float drawLine(float2 uv, float2 a, float2 b) {
    float lineF = smoothstep(0.014, 0.01, distLine(uv, a, b));
    float dist = length(b - a);
    return lineF * (smoothstep(1.3, 0.8, dist) * 0.5 + smoothstep(0.04, 0.03, abs(dist - 0.75)));
}

float n21(float2 i) {
    i += frac(i * float2(223.64, 823.12));
    i += dot(i, i + 23.14);
    return frac(i.x * i.y);
}

float2 n22(float2 i) {
    float x = n21(i);
    return float2(x, n21(i + x));
}

float2 getPoint(float2 id, float2 offset, float iTime) {
    return offset + sin(n22(id + offset) * iTime * 1.0) * 0.4;
}

float layer(float2 uv, float iTime) {
    float m = 0.0;
    float t = iTime * 2.0;

    float2 gv = frac(uv) - 0.5;
    float2 id = floor(uv) - 0.5;

    float2 p[9];
    int i = 0;
    for (float y = -1.0; y <= 1.0; y++) {
        for (float x = -1.0; x <= 1.0; x++) {
            p[i++] = getPoint(id, float2(x, y), iTime);
        }
    }

    for (int j = 0; j < 9; j++) {
        m += drawLine(gv, p[4], p[j]);
        float sparkle = 1.0 / pow(length(gv - p[j]), 1.5) * 0.005;
        m += sparkle * (sin(t + frac(p[j].x) * 12.23) * 0.4 + 0.6);
    }

    m += drawLine(gv, p[1], p[3]);
    m += drawLine(gv, p[1], p[5]);
    m += drawLine(gv, p[7], p[3]);
    m += drawLine(gv, p[7], p[5]);

    return m;
}

float4 PS_Main(PS_INPUT IN, uint pid : SV_PrimitiveID) : SV_Target
{
    float iTime = CB_GlobalConstants.Time.y;
    float2 fragCoord = IN.Position;
    float2 iResolution = float2(1920, 1080);
    float2 uv = (fragCoord - 0.5 * iResolution.xy) / iResolution.y;
    float3 c = sin(iTime * 2.0 * float3(.234, .324, .768)) * 0.4 + 0.6;
    float3 col = float3(0.0, 0.0, 0.0);

    c.x += (uv.x + 0.5);
    col += pow(-uv.y + 0.5, 5.0) * c;

    float m = 0.0;
    float x = sin(iTime * 0.1);
    float y = cos(iTime * 0.2);

    float2x2 rotMat = float2x2(x, y, -y, x);
    uv = mul(rotMat, uv);

    for (float i = 0.0; i <= 1.0; i += 1.0 / 4.0) {
        float z = frac(i + iTime * 0.05);
        float size = lerp(15.0, .1, z) * 1.50;
        float fade = smoothstep(0.0, 1.0, z) * smoothstep(1.0, 0.9, z);
        m += layer((size * uv) + i * 10.0, iTime) * fade;
    }

    col += m * c;
    return float4(col, 1.0);



    //return float4(sin(IN.Position.x) / 2 + 0.5, cos(IN.Position.y) / 2 + 0.5, sin(IN.Position.z + 2) / 2 + 0.5, 0);
    return float4(pid / 12.0f, pid / 12.0 + 0.3f, pid / 12.0 + 0.6f, 1.0);
    return IN.Color * 2.5 ;
}

