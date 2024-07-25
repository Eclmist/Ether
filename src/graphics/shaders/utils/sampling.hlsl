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

float3 TangentToWorld(float3 v, float3 N)
{
    const float3 upVector = abs(N.y) < 0.9999 ? float3(0, 1, 0) : float3(0, 0, 1);
    const float3 tangentX = normalize(cross(upVector, N));
    const float3 tangentY = cross(N, tangentX);
    // Tangent to world space
    return tangentX * v.x + tangentY * v.y + N * v.z;
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

float2 SampleSphericalMap(float3 direction)
{
    float2 uv = float2(atan2(direction.z, direction.x), asin(-direction.y));
    uv *= float2(InvPi2, InvPi);
    uv += 0.5;
    return uv;
}

