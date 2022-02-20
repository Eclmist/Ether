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
#define INV_PI      0.31830988618

float Sqr(float v) { return v * v; }
// Ref: http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html
uint ReverseBits32(uint bits)
{
#if (SHADER_TARGET >= 45)
    return reversebits(bits);
#else
    bits = (bits << 16) | (bits >> 16);
    bits = ((bits & 0x00ff00ff) << 8) | ((bits & 0xff00ff00) >> 8);
    bits = ((bits & 0x0f0f0f0f) << 4) | ((bits & 0xf0f0f0f0) >> 4);
    bits = ((bits & 0x33333333) << 2) | ((bits & 0xcccccccc) >> 2);
    bits = ((bits & 0x55555555) << 1) | ((bits & 0xaaaaaaaa) >> 1);
    return bits;
#endif
}

float VanDerCorputBase2(uint i)
{
    return ReverseBits32(i) * rcp(4294967296.0); // 2^-32
}

float2 Hammersley2dSeq(uint i, uint sequenceLength)
{
    return float2(float(i) / float(sequenceLength), VanDerCorputBase2(i));
}

// Loads elements from one of the precomputed tables for sample counts of 16, 32, 64, 256.
// Computes sample positions at runtime otherwise.
float2 Hammersley(uint i, uint sampleCount)
{
    return Hammersley2dSeq(i, sampleCount);
}



Texture2D AlbedoTex : register(t0);
Texture2D NormalTex : register(t1);
Texture2D PositionTex : register(t2);

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
    float3 m_Position;
    float3 m_Color;
    float m_Radius;
    float m_Intensity;
};

float3 Lambert(float3 albedo)
{
    return albedo * INV_PI;
}


float G1(float nDotV, float k)
{
    return nDotV / (nDotV * (1 - k) + k);
}

float G(float r, float nDotL, float nDotV)
{
    float k = Sqr(r + 1) / 8;
    return G1(nDotL, k) * G1(nDotV, k);
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

float3 FresnelSchlick(float3 f0, float nDotV)
{
    return f0 + (1 - f0) * pow(1 - nDotV, 5.0);
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

float3 ImportanceSampleGGX( float2 Xi, float Roughness, float3 N )
{
    float a = Roughness * Roughness;
    float Phi = 2 * PI * Xi.x;
    float CosTheta = sqrt( (1 - Xi.y) / ( 1 + (a*a - 1) * Xi.y ) );
    float SinTheta = sqrt( 1 - CosTheta * CosTheta );
    float3 H;
    H.x = SinTheta * cos( Phi );
    H.y = SinTheta * sin( Phi );
    H.z = CosTheta;
    float3 UpVector = abs(N.z) < 0.999 ? float3(0,0,1) : float3(1,0,0);
    float3 TangentX = normalize( cross( UpVector, N ) );
    float3 TangentY = cross( N, TangentX );
    // Tangent to world space
    return TangentX * H.x + TangentY * H.y + N * H.z;
}


float3 SpecularIBL( float3 SpecularColor , float Roughness, float3 N, float3 V )
{
    float3 SpecularLighting = 0;
    const uint NumSamples = 256;
    for( uint i = 0; i < NumSamples; i++ )
    {
        V = -V;
        float2 Xi = Hammersley( i, NumSamples );
        float3 H = ImportanceSampleGGX( Xi, Roughness, N );
        float3 L = 2 * dot( V, H ) * H - V;
        float NoV = saturate( dot( N, V ) );
        float NoL = saturate( dot( N, L ) );
        float NoH = saturate( dot( N, H ) );
        float VoH = saturate( dot( V, H ) );

        if( true )
        {
            float skyConst = 0.5;
            float3 zenithColor = float3(0.45, 0.59, 0.95) * skyConst;
            float3 horizonColor = float3(205, 220, 253) / 255.0 * skyConst;
            float3 groundColor = float3(0.379, 0.38, 0.74)* skyConst;

            float viewY = L.y;
            float3 SampleColor;

            if (viewY < 0)
                SampleColor = lerp( horizonColor, groundColor, saturate(-viewY * 2));
            else
                SampleColor = lerp(horizonColor, zenithColor, saturate(viewY * 2.0));

            // float3 SampleColor = EnvMap.SampleLevel( EnvMapSampler , L, 0 ).rgb;
            // float g = G(Roughness, NoL, NoV );
            // float Fc = pow( 1 - VoH, 5 );
            // float3 F = FresnelSchlick(0.0337f, NoV);
            // Incident light = SampleColor * NoL
            // Microfacet specular = D*G*F / (4*NoL*NoV)
            // pdf = D * NoH / (4 * VoH)
            const float alpha = Sqr(saturate(Roughness));
            const float alphaSqr = Sqr(alpha); 
            const float3 D = DistributionGGX(alphaSqr, NoH);
            float Fc = pow( 1 - VoH, 5 );
            float3 F = (1 - Fc) * SpecularColor + Fc;
            const float3 G = VisibilitySmith(alphaSqr, NoL, NoV);

            SpecularLighting += SampleColor * NoL * G * F * 4 * VoH / NoH;
        }
    }
    return SpecularLighting / NumSamples;
}

float3 ComputePointLight(float2 uv, PointLight light)
{
    const float3 pos = PositionTex.Sample(g_PointSampler, uv).xyz;
    const float3 normal = NormalTex.Sample(g_PointSampler, uv).xyz;
    const float4 albedo = AlbedoTex.Sample(g_PointSampler, uv).xyzw;
    const float roughness = albedo.w;

    const float3 l = normalize(light.m_Position - pos);
    const float3 v = normalize(g_CommonConstants.EyePosition.xyz - pos);
    const float3 h = normalize(l + v);
    const float3 n = normalize(normal);
 
    const float nDotL = saturate(dot(n, l));
    const float nDotV = saturate(dot(n, v));
    const float nDotH = saturate(dot(n, h));
    const float vDotH = saturate(dot(v, h));

    const float d = length(light.m_Position - pos);

    //https://imdoingitwrong.wordpress.com/2011/01/31/light-attenuation/    
    const float denom = d / light.m_Radius + 1.0;
    const float attenuation = 1 / (denom * denom);
    const float3 LightCol = light.m_Color * light.m_Intensity * attenuation;

    return LightCol * (Microfacet(roughness, 0.0337f, nDotL, nDotV, nDotH, vDotH) + Lambert(albedo.xyz)) * saturate(nDotL);
}

float3 ComputePointLightOld(float2 uv, PointLight light)
{
    const float3 pos = PositionTex.Sample(g_PointSampler, uv).xyz;
    const float3 col = AlbedoTex.Sample(g_PointSampler, uv).xyz;
    const float3 norm = NormalTex.Sample(g_PointSampler, uv).xyz;

    float3 dir = light.m_Position - pos;
    const float d = length(dir);
    dir = normalize(dir);
    float3 L;

    //https://imdoingitwrong.wordpress.com/2011/01/31/light-attenuation/    
    const float denom = d / light.m_Radius + 1.0;
    const float attenuation = 1 / (denom * denom);
    const float3 LightCol = light.m_Color * light.m_Intensity * attenuation * 0.4;

    const float ndotl = saturate(dot(norm, dir));
    const float3 diffuse = 1;

    const float3 r = -reflect(dir, norm);
    const float3 v = normalize(g_CommonConstants.EyePosition.xyz - pos);

    const float rdotv = saturate(dot(r, v));
    const float n = 700.2;
    const float3 specular = ndotl * pow(rdotv, n) * 10.0;

    return LightCol * (diffuse + specular) * ndotl;
}


float3 ComputeSkyLight(float2 uv)
{
    const float skyConst = 0.5;
    const float4 horizonColor = float4(255, 220, 213, 255) / 255.0 * skyConst;
    return horizonColor.xyz * 0.05 * 0;
}

float4 PS_Main(VS_OUTPUT IN) : SV_Target
{
    PointLight lights[7];

    const float4 time = g_CommonConstants.Time * 1.0;

    lights[6].m_Position = float3(sin(time.z+2.9) * 2, -7.0, cos(time.z+ 5.2) * 2);
    lights[6].m_Color = float3(0.3, 0.1, 0.3);
    lights[6].m_Intensity = 6.0f;
    lights[6].m_Radius = 10.0f;

    lights[1].m_Position = float3(10.0, sin(time.z) * 2.0, cos(time.y+10.3)* 6.0);
    lights[1].m_Color = float3(saturate(sin(time.w+0.12)), 0.0, .3);
    lights[1].m_Intensity = 2.0f;
    lights[1].m_Radius = 20.0f;

    lights[2].m_Position = float3(sin(time.y + 0.34) * 10.0, 9.0, 14.0);
    lights[2].m_Color = float3(0.3, 0.0, 1.0);
    lights[2].m_Intensity = 4.0f;
    lights[2].m_Radius = 50.0f;

    lights[3].m_Position = float3(-10.0, 9.0, sin(time.y+ 1.4) * 4.0);
    lights[3].m_Color = float3(0.6, saturate(sin(time.y + 0.5)), 0.1);
    lights[3].m_Intensity = 4.0f;
    lights[3].m_Radius = 30.0f;

    lights[4].m_Position = float3(-2.0, -6.0, -14.0);
    lights[4].m_Color = float3(0.15, 0.83, 0.57);
    lights[4].m_Intensity = 8.0f;
    lights[4].m_Radius = 40.0f;

    lights[5].m_Position = float3(4.5, 9.0, 16.0);
    lights[5].m_Color = float3(saturate(sin(time.y+1.6)), 0.73, 0.57);
    lights[5].m_Intensity = 2.0f;
    lights[5].m_Radius = 20.0f;

    //lights[0].m_Position = float3(-60.5, 0.0, 0.0);
    //lights[0].m_Color = float3(0.7, 0.78, 0.92);
    //lights[0].m_Intensity = 30.0f;
    //lights[0].m_Radius = 100.0f;

    lights[0].m_Position = float3(1.0, 2.0, 1.0) * 10.0;
    lights[0].m_Color = float3(1.0, 1.0, 1.0);
    lights[0].m_Intensity = 1.0f;
    lights[0].m_Radius = 500.0f;

    const float4 albedo = AlbedoTex.Sample(g_PointSampler, IN.UV).xyzw;
    const float3 pos = PositionTex.Sample(g_PointSampler, IN.UV).xyz;
    const float3 normal = NormalTex.Sample(g_PointSampler, IN.UV).xyz;
    const float roughness = albedo.w;

    const float3 v = normalize(g_CommonConstants.EyePosition.xyz - pos);
    const float3 n = normalize(normal);

    float3 ibl = SpecularIBL(1, roughness, n, v);
    float3 finalColor = ibl * (1.0f + albedo.xyz);

    for (int i = 0; i < 1; ++i)
    {
        //finalColor += ComputePointLightOld(IN.UV, lights[i]);
        finalColor += ComputePointLight(IN.UV, lights[i]);
    }

    // fog
    // finalColor = lerp(finalColor, float3(0.07, 0.06, 0.077), saturate(PositionTex.Sample(g_PointSampler, IN.UV).w / 100.0));

    return ComputeSkyLight(IN.UV).xyzz + finalColor.xyzz;

}
