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

#define PI          3.14159265359
#define PI2         6.28318530718
#define INV_PI      0.31830988618

#define NUM_SAMPLES 1

struct GBufferIndices
{
    uint m_AlbedoTextureIndex;
    uint m_SpecularTextureIndex;
    uint m_NormalTextureIndex;
    uint m_PositionTextureIndex;
    uint m_EnvironmentHdriTextureIndex;
};

ConstantBuffer<GBufferIndices> m_GBufferIndices : register(b1);

float2 SampleSphericalMap(float3 direction)
{
    float2 uv = float2(0.5 + atan2(direction.z,direction.x)/(PI*2), 0.5 - asin(direction.y)/PI);
    uv.x = -uv.x;
    return uv;
}

float Sqr(float v) { return v * v; }

// Ref: http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html
uint ReverseBits32(uint bits)
{
    bits = (bits << 16) | (bits >> 16);
    bits = ((bits & 0x00ff00ff) << 8) | ((bits & 0xff00ff00) >> 8);
    bits = ((bits & 0x0f0f0f0f) << 4) | ((bits & 0xf0f0f0f0) >> 4);
    bits = ((bits & 0x33333333) << 2) | ((bits & 0xcccccccc) >> 2);
    bits = ((bits & 0x55555555) << 1) | ((bits & 0xaaaaaaaa) >> 1);
    return bits;
}

float VanDerCorputBase2(uint i)
{
    return ReverseBits32(i) * rcp(4294967296.0); // 2^-32
}

float2 Hammersley(uint i, uint sequenceLength)
{
    return float2(float(i) / float(sequenceLength), VanDerCorputBase2(i));
}

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
    float3 Position;
    float3 Color;
    float Radius;
    float Intensity;
};

struct Material
{
    float3 BaseColor;
    float3 SpecularColor;
    float Roughness;
    float Metalness;
};

float3 Lambert(float3 albedo)
{
    return albedo * INV_PI;
}

float3 VisibilitySmith(float alphaSqr, float nDotL, float nDotV)
{
    const float nDotLSqr = Sqr(nDotL);
    const float nDotVSqr = Sqr(nDotV);
    const float visSmithL = nDotL + sqrt(nDotLSqr * (1.0f - alphaSqr) + alphaSqr);
    const float visSmithV = nDotV + sqrt(nDotVSqr * (1.0f - alphaSqr) + alphaSqr);
    return rcp(visSmithL * visSmithV);
}

float VisibilitySmithJointApprox(float alpha, float nDotL, float nDotV)
{
    const float visSmithL = nDotL * (nDotV * (1.0f - alpha) + alpha);
    const float visSmithV = nDotV * (nDotL * (1.0f - alpha) + alpha);
    return 0.5 * rcp(visSmithL + visSmithV);
}

float3 DistributionGGX(float alphaSqr, float nDotH)
{
    const float cosThetaSqr = Sqr(nDotH);
    return INV_PI * alphaSqr / Sqr((alphaSqr - 1.0f) * cosThetaSqr + 1.0f);
}

float3 FresnelSchlick(float3 f0, float vDotH)
{
    return f0 + (1 - f0) * pow(1 - vDotH, 5.0);
}

float3 Microfacet(float roughness, float3 f0, float nDotL, float nDotV, float nDotH, float vDotH)
{
    const float alpha = Sqr(saturate(roughness));
    const float alphaSqr = Sqr(alpha);

    const float3 D = DistributionGGX(alphaSqr, nDotH);
    const float3 F = FresnelSchlick(f0, vDotH);
    const float3 V = VisibilitySmith(alphaSqr, nDotL, nDotV);

    return D * F * V;
}

float3 Ue4Brdf(float3 wi, float3 wo, float3 normal, Material material)
{
    const float3 l = wi;
    const float3 v = wo;
    const float3 n = normal;
    const float3 h = normalize(l + v);
 
    const float nDotL = saturate(dot(n, l));
    const float nDotV = saturate(dot(n, v));
    const float nDotH = saturate(dot(n, h));
    const float vDotH = saturate(dot(v, h));

    float3 f0 = lerp(0.08 * material.SpecularColor, material.BaseColor, material.Metalness);
    return Lambert(material.BaseColor) * (1.0 - material.Metalness) + Microfacet(material.Roughness, f0, nDotL, nDotV, nDotH, vDotH);
}

float3 ImportanceSampleGGX( float2 Xi, float Roughness, float3 N )
{
    float a = Roughness * Roughness;
    float phi = 2 * PI * Xi.x;
    float cosTheta = sqrt( (1 - Xi.y) / ( 1 + (a*a - 1) * Xi.y ) );
    float sinTheta = sqrt( 1 - cosTheta * cosTheta );
    float3 H;
    H.x = sinTheta * cos( phi );
    H.y = sinTheta * sin( phi );
    H.z = cosTheta;
    float3 UpVector = N.y < 0.9999 ? float3(0,1,0) : float3(0,0,1);
    float3 TangentX = normalize( cross(UpVector, N) );
    float3 TangentY = cross( N, TangentX );
    // Tangent to world space
    return TangentX * H.x + TangentY * H.y + N * H.z;
}

float3 UniformSample(float2 Xi, float3 N)
{
    float phi = Xi.y * 2.0 * PI;
    float cosTheta = 1.0 - Xi.x;
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);
    float3 H;
    H.x = sinTheta * cos( phi );
    H.y = sinTheta * sin( phi );
    H.z = cosTheta;    
    float3 UpVector = N.y < 0.9999 ? float3(0,1,0) : float3(0,0,1);
    float3 TangentX = normalize( cross(UpVector, N) );
    float3 TangentY = cross( N, TangentX );
    // Tangent to world space
    return TangentX * H.x + TangentY * H.y + N * H.z;
}

float3 SpecularIBL(float3 normal, float3 wo, Material material)
{
    float3 SpecularLighting = 0;
    for( uint i = 0; i < NUM_SAMPLES; i++ )
    {
        float2 Xi = Hammersley( i, NUM_SAMPLES );
        float3 H = normalize(ImportanceSampleGGX( Xi, material.Roughness, normal ));
        float3 wi = normalize(2 * dot( wo, H ) * H - wo);
        float NoV = saturate(dot(normal, wo));
        float NoL = saturate(dot(normal, wi));
        float NoH = saturate(dot(normal, H));
        float VoH = saturate(dot(wo, H));

        if (NoL > 0)
        {
            const float alpha = Sqr(saturate(material.Roughness));
            const float alphaSqr = Sqr(alpha);
            const float3 D = DistributionGGX(alphaSqr, NoH);

            Texture2D hdriTex = ResourceDescriptorHeap[m_GBufferIndices.m_EnvironmentHdriTextureIndex];
            float3 Li = hdriTex.Sample(g_BilinearSampler, SampleSphericalMap(wi)).rgb;
            float3 f0 = lerp(0.08 * material.SpecularColor, material.BaseColor, material.Metalness);
            float3 brdf = Microfacet(material.Roughness, f0, NoL, NoV, NoH, VoH);
            float3 pdfInv = (4 * VoH) / (D * NoH);
            SpecularLighting += Li * NoL * brdf * pdfInv;
        }
   }

    return SpecularLighting / NUM_SAMPLES;
}

float3 DiffuseIBL(float3 normal, float3 wo, Material material)
{
    float3 DiffuseLighting = 0;
    for( uint i = 0; i < NUM_SAMPLES; i++ )
    {
        float2 Xi = Hammersley( i, NUM_SAMPLES );
        float3 wi = normalize(UniformSample( Xi, normal ));
        wi = dot(normal, wi) < 0.0f ? -wi : wi;
        float NoL = saturate(dot(normal, wi));

        Texture2D hdriTex = ResourceDescriptorHeap[m_GBufferIndices.m_EnvironmentHdriTextureIndex];
        float3 Li = hdriTex.Sample(g_BilinearSampler, SampleSphericalMap(wi)).rgb;
        float3 brdf = Lambert(material.BaseColor) * (1.0 - material.Metalness);

        // return Li * brdf;
        // return Li * NoL;
        // return Li;

        DiffuseLighting += Li * NoL * PI2 * brdf;// * NoL * brdf;
    }
    return DiffuseLighting / NUM_SAMPLES;
}

float3 ComputePointLight(float2 uv, float3 pos, float3 normal, Material material, PointLight light)
{
    const float3 wi = normalize(light.Position - pos);
    const float3 wo = normalize(g_CommonConstants.EyePosition.xyz - pos);
    const float3 n = normalize(normal);
 
    const float d = length(light.Position - pos);
    const float denom = d / light.Radius + 1.0;
    float fAtt = 1 / (denom * denom);

    const float3 L = light.Color * light.Intensity * fAtt;

    return L * Ue4Brdf(wi, wo, n, material) * saturate(dot(n, wi));
}

float4 PS_Main(VS_OUTPUT IN) : SV_Target
{

    PointLight lights[1];

    const float4 time = g_CommonConstants.Time;
{

    //lights[6].Position = float3(sin(time.z+2.9) * 2, -7.0, cos(time.z+ 5.2) * 2);
    //lights[6].Color = float3(0.3, 0.1, 0.3);
    //lights[6].Intensity = 6.0f;
    //lights[6].Radius = 10.0f;

    //lights[1].Position = float3(10.0, sin(time.z) * 2.0, cos(time.y+10.3)* 6.0);
    //lights[1].Color = float3(saturate(sin(time.w+0.12)), 0.0, .3);
    //lights[1].Intensity = 2.0f;
    //lights[1].Radius = 20.0f;

    //lights[2].Position = float3(sin(time.y + 0.34) * 10.0, 9.0, 14.0);
    //lights[2].Color = float3(0.3, 0.0, 1.0);
    //lights[2].Intensity = 4.0f;
    //lights[2].Radius = 50.0f;

    //lights[3].Position = float3(-10.0, 9.0, sin(time.y+ 1.4) * 4.0);
    //lights[3].Color = float3(0.6, saturate(sin(time.y + 0.5)), 0.1);
    //lights[3].Intensity = 4.0f;
    //lights[3].Radius = 30.0f;

    //lights[4].Position = float3(-2.0, -6.0, -14.0);
    //lights[4].Color = float3(0.15, 0.83, 0.57);
    //lights[4].Intensity = 8.0f;
    //lights[4].Radius = 40.0f;

    //lights[5].Position = float3(4.5, 9.0, 16.0);
    //lights[5].Color = float3(saturate(sin(time.y+1.6)), 0.73, 0.57);
    //lights[5].Intensity =  5.0f;
    //lights[5].Radius = 20.0f;

    lights[0].Position = float3(-5.5, 0.0, 0.0);
    lights[0].Color = float3(0.7, 0.78, 0.92);
    lights[0].Intensity = 1.0f;

    lights[0].Radius = 100.0f;

    //lights[0].Position = float3(5.0, 2.0, 0.0);
    //lights[0].Color = float3(1.0, 1.0, 1.0);
    //lights[0].Intensity = 10.0f;// (sin(time.z) * 0.5 + 0.5) * 10.0f;
    //lights[0].Radius = 3.0f;
}


    Texture2D m_AlbedoTex = ResourceDescriptorHeap[m_GBufferIndices.m_AlbedoTextureIndex];
    Texture2D m_SpecularTex = ResourceDescriptorHeap[m_GBufferIndices.m_SpecularTextureIndex];
    Texture2D m_PositionTex = ResourceDescriptorHeap[m_GBufferIndices.m_PositionTextureIndex];
    Texture2D m_NormalTex = ResourceDescriptorHeap[m_GBufferIndices.m_NormalTextureIndex];


    float4 albedo = m_AlbedoTex.Sample(g_PointSampler, float2(IN.UV.x, IN.UV.y));
    float4 specular = m_SpecularTex.Sample(g_PointSampler, float2(IN.UV.x, IN.UV.y));
    float4 pos = m_PositionTex.Sample(g_PointSampler, float2(IN.UV.x, IN.UV.y));
    float3 normal = m_NormalTex.Sample(g_PointSampler, float2(IN.UV.x, IN.UV.y)).xyz;

    Material mat;
    mat.BaseColor = albedo.xyz;
    mat.SpecularColor = specular.xyz;
    mat.Roughness = albedo.w;
    mat.Metalness = specular.w;

    const float3 v = normalize(g_CommonConstants.EyePosition.xyz - pos);
    const float3 n = normalize(normal);
    float3 finalColor = 0;// DiffuseIBL(n, v, mat) + SpecularIBL(n, v, mat);

    for (int i = 0; i < 1; ++i)
    {
        lights[i].Radius *= 0.7;
        lights[i].Position *= 5.2;
        lights[i].Intensity *= 4.9;
        lights[i].Color = 1.0;
        finalColor += ComputePointLight(IN.UV, pos, normal, mat, lights[i]);
    }

    // fog
    finalColor = lerp(finalColor, float3(0.05, 0.05, 0.04) * 2, saturate(pos.w / 100.0));

    return float4(finalColor, 1.0f);

}
