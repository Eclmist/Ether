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

#include "utils/noise.hlsl"
#include "utils/constants.hlsl"

// A Low Distortion Map Between Disk and Square
// By Peter Shirley and Kenneth Chiu
float2 SquareToConcentricDiskMapping(float2 uv)
{
    float phi = 0.0f;
    float r = 0.0f;

    float a = 2.0f * uv.x - 1.0f;
    float b = 2.0f * uv.y - 1.0f;

    if (a > -b)
    {
        if (a > b)
        {
            r = a;
            phi = (Pi / 4.0f) * (b / a);
        }
        else
        {
            r = b;
            phi = (Pi / 4.0f) * (2.0f - (a / b));
        }
    }
    else
    {
        if (a < b)
        {
            r = -a;
            phi = (Pi / 4.0f) * (4.0f + (b / a));
        }
        else
        {
            r = -b;
            if (b != 0)
                phi = (Pi / 4.0f) * (6.0f - (a / b));
            else
                phi = 0;
        }
    }

    return float2(r * cos(phi), r * sin(phi));
}

float3 SampleDirectionSphere(float2 uv)
{
    float z = uv.x * 2.0f - 1.0f;
    float r = sqrt(max(0.0f, 1.0f - z * z));
    float phi = 2.0 * Pi * uv.y;
    float x = r * cos(phi);
    float y = r * sin(phi);
    return float3(x, y, z);
}

float3 SampleDirectionHemisphere(float2 uv)
{
    float z = uv.x;
    float r = sqrt(max(0.0f, 1.0f - z * z));
    float phi = 2.0f * Pi * uv.y;
    float x = r * cos(phi);
    float y = r * sin(phi);
    return float3(x, y, z);
}

float3 SampleDirectionCosineHemisphere(float2 uv)
{
    uv = SquareToConcentricDiskMapping(uv);
    float3 dir;
    float r = uv.x * uv.x + uv.y * uv.y;
    dir.x = uv.x;
    dir.y = uv.y;
    dir.z = sqrt(max(0.0f, 1.0f - r));
    return dir;
}

float SampleDirectionSphere_Pdf()
{
    return 1.0f / (Pi * 4.0f);
}

float SampleDirectionHemisphere_Pdf()
{
    return 1.0f / (Pi * 2.0f);
}

float SampleDirectionCosineHemisphere_Pdf(float cosTheta)
{
    return cosTheta / Pi;
}


float CMJ_Random_Internal(uint i, uint p)
{
    i ^= p;
    i ^= i >> 17;
    i ^= i >> 10;
    i *= 0xb36534e5;
    i ^= i >> 12;
    i ^= i >> 21;
    i *= 0x93fc4795;
    i ^= 0xdf6e307f;
    i ^= i >> 17;
    i *= 1 | p >> 18;
    return i * (1.0f / 4294967808.0f);
}

uint CMJ_Permute_Internal(uint i, uint l, uint p)
{
    uint w = l - 1;
    w |= w >> 1;
    w |= w >> 2;
    w |= w >> 4;
    w |= w >> 8;
    w |= w >> 16;
    do
    {
        i ^= p;
        i *= 0xe170893d;
        i ^= p >> 16;
        i ^= (i & w) >> 4;
        i ^= p >> 8;
        i *= 0x0929eb3f;
        i ^= p >> 23;
        i ^= (i & w) >> 1;
        i *= 1 | p >> 27;
        i *= 0x6935fa69;
        i ^= (i & w) >> 11;
        i *= 0x74dcb303;
        i ^= (i & w) >> 2;
        i *= 0x9e501cc3;
        i ^= (i & w) >> 2;
        i *= 0xc860a3df;
        i &= w;
        i ^= i >> 5;
    } while (i >= l);
    return (i + p) % l;
}

float2 CMJ_Sample2D(uint idx, uint numSamplesX, uint numSamplesY, uint pattern)
{
    uint N = numSamplesX * numSamplesY;
    idx = CMJ_Permute_Internal(idx, N, pattern * 0x51633e2d);
    uint sx = CMJ_Permute_Internal(idx % numSamplesX, numSamplesX, pattern * 0x68bc21eb);
    uint sy = CMJ_Permute_Internal(idx / numSamplesX, numSamplesY, pattern * 0x02e5be93);
    float jx = CMJ_Random_Internal(idx, pattern * 0x967a889b);
    float jy = CMJ_Random_Internal(idx, pattern * 0x368cc8b7);
    return float2((sx + (sy + jx) / numSamplesY) / numSamplesX, (idx + jy) / N);
}
