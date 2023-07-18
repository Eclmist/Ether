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

#define ZERO_GUARD(f) (max(0.001, f))

float Sqr(float v)
{
    return v * v;
}

float3 Lambert(float3 albedo)
{
    return albedo * InvPi;
}

// UE4's modification of GGX/Trowbridge-Reitz
float UE4_Specular_D(float alphaSqr, float nDotH)
{
    return alphaSqr / ZERO_GUARD(Pi * Sqr(Sqr(nDotH) * (alphaSqr - 1) + 1));
}

// UE4's modification of Schlick geometry term
float UE4_Specular_G(float alpha, float nDotL, float nDotV)
{
    const float k = alpha / 2.0f;
    const float G1_L = nDotL / ZERO_GUARD((nDotL * (1 - k) + k));
    const float G2_V = nDotV / ZERO_GUARD((nDotV * (1 - k) + k));
    return G1_L * G2_V;
}

// UE4's modification of Schlick's fresnel term
float3 UE4_Specular_F(float3 f0, float vDotH)
{
    const float pwr = (-5.55473 * vDotH - 6.98316) * vDotH;
    return f0 + (1 - f0) * pow(2, pwr);
}

float3 FresnelSchlick(float3 f0, float vDotH)
{
    return f0 + (1 - f0) * pow(1 - vDotH, 5.0);
}

float3 UE4_Specular(float roughness, float3 f0, float nDotL, float nDotV, float nDotH, float vDotH)
{
    const float alpha = Sqr(saturate(roughness));
    const float alphaSqr = Sqr(alpha);

    const float3 D = UE4_Specular_D(alphaSqr, nDotH);
    const float3 F = UE4_Specular_F(f0, vDotH);
    const float3 G = UE4_Specular_G(alpha, nDotL, nDotV);

    return (D * F * G) / ZERO_GUARD(4 * nDotL * nDotV);
}

float3 UE4_Brdf(float3 wi, float3 wo, float3 normal, float3 albedo, float roughness, float metalness)
{
    const float3 l = wi;
    const float3 v = wo;
    const float3 n = normal;
    const float3 h = normalize(l + v);

    const float nDotL = saturate(dot(n, l));
    const float nDotV = saturate(dot(n, v));
    const float nDotH = saturate(dot(n, h));
    const float vDotH = saturate(dot(v, h));

    const float3 f0 = lerp(0.08, albedo, metalness);
    const float3 diffuse = Lambert(albedo);
    const float3 specular = UE4_Specular(roughness, f0, nDotL, nDotV, nDotH, vDotH);
    return diffuse * (1.0 - metalness) + specular;
}

float3 ImportanceSampleGGX(float2 Xi, float roughness, float3 N)
{
    const float a = roughness * roughness;
    const float phi = 2 * Pi * Xi.x;
    const float cosTheta = sqrt((1 - Xi.y) / (1 + (a * a - 1) * Xi.y));
    const float sinTheta = sqrt(1 - cosTheta * cosTheta);
    float3 H;
    H.x = sinTheta * cos(phi);
    H.y = sinTheta * sin(phi);
    H.z = cosTheta;
    const float3 upVector = N.y < 0.9999 ? float3(0, 1, 0) : float3(0, 0, 1);
    const float3 tangentX = normalize(cross(upVector, N));
    const float3 tangentY = cross(N, tangentX);
    // Tangent to world space
    return tangentX * H.x + tangentY * H.y + N * H.z;
}

inline float GGX_PDF(float NdotH, float roughness)
{
    const float alpha = Sqr(saturate(roughness));
    const float alphaSqr = Sqr(alpha);

    return UE4_Specular_D(alphaSqr, NdotH) * NdotH;
}
