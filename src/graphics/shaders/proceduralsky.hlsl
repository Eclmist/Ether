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

#include "common/globalconstants.h"
#include "utils/fullscreenhelpers.hlsl"

ConstantBuffer<GlobalConstants> g_GlobalConstants : register(b0);

struct VS_OUTPUT
{
    float4 Position : SV_Position;
    float2 TexCoord : TEXCOORD;
};

VS_OUTPUT VS_Main(uint ID : SV_VertexID)
{
    float2 pos;
    float2 uv;
    GetVertexFromID(ID, pos, uv);

    VS_OUTPUT o;
    o.Position = float4(pos, 1.0, 1.0);
    o.TexCoord = uv;

    return o;
}

// 3D Gradient noise from: https://www.shadertoy.com/view/Xsl3Dl
float3 hash(float3 p) // replace this by something better
{
    p = float3(dot(p, float3(127.1, 311.7, 74.7)), dot(p, float3(269.5, 183.3, 246.1)), dot(p, float3(113.5, 271.9, 124.6)));

    return -1.0 + 2.0 * frac(sin(p) * 43758.5453123);
}
float noise(float3 p)
{
    float3 i = floor(p);
    float3 f = frac(p);

    float3 u = f * f * (3.0 - 2.0 * f);

    return lerp(
        lerp(lerp(dot(hash(i + float3(0.0, 0.0, 0.0)), f - float3(0.0, 0.0, 0.0)),
                dot(hash(i + float3(1.0, 0.0, 0.0)), f - float3(1.0, 0.0, 0.0)),
                u.x),
            lerp(dot(hash(i + float3(0.0, 1.0, 0.0)), f - float3(0.0, 1.0, 0.0)),
                dot(hash(i + float3(1.0, 1.0, 0.0)), f - float3(1.0, 1.0, 0.0)),
                u.x),
            u.y),
        lerp(lerp(dot(hash(i + float3(0.0, 0.0, 1.0)), f - float3(0.0, 0.0, 1.0)),
                dot(hash(i + float3(1.0, 0.0, 1.0)), f - float3(1.0, 0.0, 1.0)),
                u.x),
            lerp(dot(hash(i + float3(0.0, 1.0, 1.0)), f - float3(0.0, 1.0, 1.0)),
                dot(hash(i + float3(1.0, 1.0, 1.0)), f - float3(1.0, 1.0, 1.0)),
                u.x),
            u.y),
        u.z);
}


float Stars(float3 viewDir)
{
    float stars_threshold = 12.0f;                                   // modifies the number of stars that are visible
    float stars_exposure = 2000.0f;                                  // modifies the overall strength of the stars
    float stars = pow(clamp(noise(viewDir * 200.0f), 0.0f, 1.0f), stars_threshold) * stars_exposure;
    stars *= lerp(0.4, 1.4, noise(viewDir * 100.0f + g_GlobalConstants.m_Time.x / 100)); // time based flickering
    return saturate(stars);
}

float3 NormalizeDirection(float2 screenUV)
{
    float2 res = g_GlobalConstants.m_ScreenResolution;
    float aspect = res.x / res.y;
    float2 offset;
    offset.x = tan(1.39626 * 0.5) * (screenUV.x * 2.0 - 1.0) * aspect;
    offset.y = tan(1.39626 * 0.5) * (screenUV.y * 2.0 - 1.0);

    float3 forward = normalize(g_GlobalConstants.m_EyeDirection.xyz);
    float3 right = normalize(cross(float3(0, 1, 0), forward));
    float3 up = normalize(cross(right, forward));

    float3 dir = normalize(g_GlobalConstants.m_EyeDirection.xyz + offset.x * right + offset.y * up);
    return dir;
}

float3 CalculateSunRadiance(float3 viewDirection, float3 sunDirection, float3 sunColor)
{
    float cosTheta = dot(viewDirection, sunDirection);
    float sunIntensity = saturate(smoothstep(0.999, 1.0, cosTheta));
    float3 sunRadiance = sunColor * sunIntensity;
    return saturate(sunRadiance);
}
float3 CalculateRayleighScattering(float3 viewDirection, float3 sunDirection, float rayleighCoefficient)
{
    float cosTheta = saturate(dot(viewDirection, sunDirection));
    float cos2Theta = cosTheta * cosTheta;
    float phaseR = 3 / (16 * 3.1415) * (1 + cos2Theta);
    return phaseR * rayleighCoefficient;
}

float3 CalculateMieScattering(float3 viewDirection, float3 sunDirection, float mieCoefficient, float mieDirectionality)
{
    float cosTheta = dot(viewDirection, sunDirection);

    if (cosTheta > 0.0f)
    {
        float g = mieDirectionality;
        float phase = 1.5 * 1.0 / (4.0 * 3.1415) * (1.0 - g * g) *
                             pow(max(1.0 + (g * g) - 2.0 * g * cosTheta, 0.0001), -3.0 / 2.0) *
                             (1.0 + cosTheta * cosTheta) /
                             (2.0 + g * g);

        return mieCoefficient * phase;
    }

    return 0;
}

float3 CalculateSkyColor(float3 rayleighScattering, float3 mieScattering, float skyLuminance)
{
    // Combine the Rayleigh and Mie scattering components and adjust the luminance
    float3 skyColor = saturate(rayleighScattering) * lerp(float3(5.8, 8.5, 33.1) * 0.1, float3(0.3, 0.3, 0.7), 1 - (g_GlobalConstants.m_SunDirection.y - 0.3));
    skyColor += saturate(mieScattering) * lerp(float3(0.8, 0.3, 0.9), float3(1, 1, 1), 1 - g_GlobalConstants.m_SunDirection.y);
    skyColor *= skyLuminance;

    return skyColor;
}

float3 CalculateTurbidityCorrection(float3 skyTurbidity)
{
    // Calculate the atmospheric turbidity correction based on the sky turbidity
    float3 turbidityCorrection = float3(1.0f, 1.0f, 1.0f);

    // Adjust the turbidity correction based on the sky turbidity
    turbidityCorrection *= skyTurbidity;

    return turbidityCorrection;
}

float3 CalculateSkyRadiance(
    float3 viewDirection,
    float3 skyTurbidity,
    float skyRayleigh,
    float skyMie,
    float skyMieDirectionality,
    float skyLuminance)
{
    // Calculate atmospheric scattering components
    float3 rayleighScattering = CalculateRayleighScattering(viewDirection, g_GlobalConstants.m_SunDirection, skyRayleigh);
    float3 mieScattering = CalculateMieScattering(viewDirection, g_GlobalConstants.m_SunDirection, skyMie, skyMieDirectionality);
    float3 skyColor = CalculateSkyColor(rayleighScattering, mieScattering, skyLuminance);

    // Apply atmospheric turbidity
    float3 turbidityCorrection = CalculateTurbidityCorrection(skyTurbidity);
    skyColor *= turbidityCorrection;

    return saturate(skyColor);
}

float4 ProceduralSky(float2 texCoord)
{
    const float3 g_SkyTurbidity = float3(1, 1, 1);
    const float g_SkyRayleigh = 10.0f;
    const float g_SkyMie = 0.2f;
    const float g_SkyMieDirectionality = 0.7f;
    const float g_SkyLuminance = 1.0f;

    // Normalize screenUV to get a direction vector
    float3 viewDirection = NormalizeDirection(texCoord);

    // Calculate sun contribution
    float3 sunRadiance = CalculateSunRadiance(
        viewDirection,
        g_GlobalConstants.m_SunDirection.xyz,
        g_GlobalConstants.m_SunColor.rgb);

    // Calculate sky contribution
    float3 skyRadiance = CalculateSkyRadiance(
        viewDirection,
        g_SkyTurbidity,
        g_SkyRayleigh,
        g_SkyMie,
        g_SkyMieDirectionality,
        g_SkyLuminance);

    float3 stars = Stars(viewDirection);

    // Combine sun and sky radiance
    float3 totalRadiance = sunRadiance + skyRadiance;
    totalRadiance += lerp(stars, 0, saturate(length(totalRadiance)));
    return float4(totalRadiance, 1.0f);
}

float4 PS_Main(VS_OUTPUT IN) : SV_Target
{
    return ProceduralSky(IN.TexCoord);
}