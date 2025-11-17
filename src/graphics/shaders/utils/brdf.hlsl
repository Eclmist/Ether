/*
    This file is part of Ether, an open-source DirectX 12 renderer.

    Copyright (c) 2020-2030 Samuel Huang - All rights reserved.

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

#ifndef __BRDF_HLSL__
#define __BRDF_HLSL__

#include "utils/constants.hlsl"
#include "utils/sampling.hlsl"

float Sqr(float v)
{
    return v * v;
}

float3 Lambert(float3 albedo)
{
    return albedo * InvPi;
}

float DistributionGGX(float alphaSqr, float nDotH)
{
    alphaSqr = max(0.0001, alphaSqr);
    return alphaSqr / (Pi * Sqr(Sqr(nDotH) * (alphaSqr - 1) + 1));
}

float GeometrySchlick(float alpha, float nDotL, float nDotV)
{
    const float k = alpha / 2.0f;
    const float G1_L = nDotL / ((nDotL * (1 - k) + k));
    const float G2_V = nDotV / ((nDotV * (1 - k) + k));
    return G1_L * G2_V;
}

float3 FresnelSchlickApprox(float3 f0, float vDotH)
{
    const float pwr = (-5.55473 * vDotH - 6.98316) * vDotH;
    return f0 + (1 - f0) * pow(2, pwr);
}

float3 FresnelSchlick(float3 f0, float vDotH)
{
    return f0 + (1 - f0) * pow(1 - vDotH, 5.0);
}

float3 Microfacet(float roughness, float3 f0, float nDotL, float nDotV, float nDotH, float vDotH)
{
    const float alpha = Sqr(roughness);
    const float alphaSqr = Sqr(alpha);

    const float D = DistributionGGX(alphaSqr, nDotH);
    const float G = GeometrySchlick(alpha, nDotL, nDotV);
    const float3 F = FresnelSchlick(f0, vDotH);

    return (D * F * G) / max(0.001, (4 * nDotL * nDotV));
}

float3 BRDF_UE4(float3 wi, float3 wo, float3 normal, float3 baseColor, float roughness, float metalness)
{
    wi = normalize(wi);
    wo = normalize(wo);
    normal = normalize(normal);

    if (dot(wi, normal) < 0.0f)
        normal = -normal;
    
    roughness = min(max(0.01f, roughness), 1.0f);
    metalness = saturate(metalness);
    
    const float3 l = normalize(wi);
    const float3 v = normalize(wo);
    const float3 n = normalize(normal);
    const float3 h = normalize(l + v);

    const float nDotV = clamp(dot(n, v), 0.01f, 1);
    const float nDotL = clamp(dot(n, l), 0.01f, 1);
    const float nDotH = clamp(dot(n, h), 0.01f, 1);
    const float vDotH = clamp(dot(v, h), 0.01f, 1);

    const float3 albedo = baseColor * (1.0f - metalness);
    const float3 f0 = lerp(0.04, baseColor, metalness);
    const float3 diffuse = Lambert(albedo);
    const float3 specular = Microfacet(roughness, f0, nDotL, nDotV, nDotH, vDotH);
    return diffuse + specular;
}

float3 ImportanceSampleGGX(float2 Xi, float3 wo, float3 N, float roughness)
{
    const float a = roughness * roughness;
    const float phi = 2 * Pi * Xi.x;
    const float cosTheta = sqrt((1 - Xi.y) / (1 + (a * a - 1) * Xi.y));
    const float sinTheta = sqrt(max(0, 1 - cosTheta * cosTheta));
    float3 H;
    H.x = sinTheta * cos(phi);
    H.y = sinTheta * sin(phi);
    H.z = cosTheta;

    // Tangent to world space
    const float3 ws_H = TangentToWorld(H, N);
    const float3 wi = normalize(reflect(-wo, ws_H));
    return wi;
}

inline float DistributionGGX_Pdf(float nDotH, float vDotH, float roughness)
{
    const float alpha = Sqr(roughness);
    const float alphaSqr = Sqr(alpha);
    return vDotH <= 0 ? 0 : (DistributionGGX(alphaSqr, nDotH) * nDotH) / (4 * vDotH);
}

// Pdf of sampling both diffuse and specular terms of a joint BRDF like
// UE4's BRDF.
// t: t is the probability of sampling the specular term,
//    whereas 1 - t is the probability of sampling the diffuse term.
//    This maps to "specularWeight" in raytracedlights.hlsl
inline float UE4JointPdf(float t, float nDotH, float nDotL, float vDotH, float roughness)
{
    const float pdf_d = nDotL / Pi;
    const float pdf_s = DistributionGGX_Pdf(nDotH, vDotH, roughness);
    return ((1 - t) * pdf_d) + (t * pdf_s);
}

#endif // __BRDF_HLSL__
