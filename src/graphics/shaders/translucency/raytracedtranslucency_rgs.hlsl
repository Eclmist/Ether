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

#ifndef __PATHTRACING_RGS_HLSL__
#define __PATHTRACING_RGS_HLSL__


#include "utils/helpers.hlsl"
#include "utils/sampling.hlsl"
#include "utils/fullscreenhelpers.hlsl"

#include "common/globalconstants.h"
#include "common/raytracingconstants.h"
#include "utils/encoding.hlsl"
#include "utils/shading.hlsl"
#include "utils/brdf.hlsl"

Texture2D<float> g_SceneDepth                       : register(t3);
RWTexture2D<float4> g_SceneColor                    : register(u0);

// TODO: Remove duplicates

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

float3 SampleEnvironmentLighting(float3 wi, float mipLevel)
{
    sampler linearSampler = SamplerDescriptorHeap[g_GlobalConstants.m_SamplerIndex_Linear_Wrap];
    Texture2D<float4> hdriTexture = ResourceDescriptorHeap[g_GlobalConstants.m_HdriTextureIndex];
    const float exposure = g_GlobalConstants.m_SkyIntensity;

    const float2 hdriUv = SampleSphericalMap(wi);
    const float4 hdri = hdriTexture.SampleLevel(linearSampler, hdriUv, mipLevel);
    const float sunsetFactor = saturate(asin(dot(g_GlobalConstants.m_SunDirection.xyz, float3(0, 1, 0))));
    const float sunlightFactor = 1 - saturate(asin(dot(g_GlobalConstants.m_SunDirection.xyz, float3(0, -1, 0))));

    const float4 color = lerp(float4(0.5, 0.25, 0.25, 0), 1, sunsetFactor) * sunlightFactor;

    return (exposure * hdri * color).xyz;
}


#define NUM_SAMPLES 4

float3 BruteForcedIBL(ShadingSurface surface, float3 wo)
{
    sampler linearSampler = SamplerDescriptorHeap[g_GlobalConstants.m_SamplerIndex_Linear_Wrap];

    float3 ibl = 0;
    for (uint i = 0; i < NUM_SAMPLES; i++)
    {
        float2 Xi = Hammersley(i, NUM_SAMPLES);
        float3 H = normalize(ImportanceSampleGGX(Xi, wo, surface.m_Normal, surface.m_Roughness));
        float3 wi = normalize(2 * dot(wo, H) * H - wo);
        float NoV = saturate(dot(surface.m_Normal, wo));
        float NoL = saturate(dot(surface.m_Normal, wi));
        float NoH = saturate(dot(surface.m_Normal, H));
        float VoH = saturate(dot(wo, H));

        if (NoL > 0)
        {
            const float3 Li = SampleEnvironmentLighting(wi, 8);
            const float3 f = BRDF_UE4(wi, wo, surface.m_Normal, surface.m_BaseColor, surface.m_Roughness, surface.m_Metalness);

            ibl += Li * f * NoL;
        }
    }

    return ibl / NUM_SAMPLES;
}

float3 ComputeRadiance(ShadingSurface surface, float3 Li, float3 wi, float3 wo)
{
    wi = normalize(wi);
    wo = normalize(wo);
    const float3 f = BRDF_UE4(wi, wo, surface.m_Normal, surface.m_BaseColor, surface.m_Roughness, surface.m_Metalness);
    const float cosTheta = saturate(dot(wi, surface.m_Normal));
    return f * Li * cosTheta;
}

void SampleDirectionBrdf(ShadingSurface surface, float seed, float3 wo, out float3 wi, out float pdf)
{
    wo = normalize(wo);
    
    const uint2 sampleCoords = DispatchRaysIndex().xy;
    const uint2 bufferSize = DispatchRaysDimensions().xy;
    const uint sampleIdx = sampleCoords.y * bufferSize.x + sampleCoords.x;
    const float2 rand2D = CMJ_Sample2D(sampleIdx, 1024, 1024, seed);

    const float diffuseWeight = lerp(lerp(0.5, 1.0, surface.m_Roughness), 0.0, surface.m_Metalness);
    const float specularWeight = 1.0 - diffuseWeight;

    if (Random(rand2D.x) <= specularWeight)
        wi = normalize(ImportanceSampleGGX(rand2D, wo, surface.m_Normal, surface.m_Roughness));
    else
        wi = normalize(TangentToWorld(SampleDirectionCosineHemisphere(rand2D), surface.m_Normal));

    const float3 H = normalize(wi + wo);
    const float nDotH = saturate(dot(surface.m_Normal, H));
    const float nDotV = saturate(dot(surface.m_Normal, wo));
    const float vDotH = saturate(dot(wo, H));
    const float cosTheta = abs(dot(wi, surface.m_Normal));

    pdf = UE4JointPdf(specularWeight, nDotH, cosTheta, vDotH, surface.m_Roughness);
}

float3 SampleEnvironmentLighting(float3 wi)
{
    return SampleEnvironmentLighting(wi, 4);
}

TranslucentRayPayload TracePrimaryRay(float2 screenCoords, float sceneDepth, uint depth)
{
    const float3 worldPos = ScreenToWorldSpace(screenCoords, sceneDepth);
    const float3 origin = g_GlobalConstants.m_CameraPosition.xyz;
    const float3 direction = normalize(worldPos - origin);
    const float tmax = length(worldPos - origin) * 0.99;

    TranslucentRayPayload payload;
    payload.m_Depth = depth;
    payload.m_Radiance = 0;
    payload.m_Flags = 0;
    payload.SetPrimaryRay(true);

    RayDesc ray;
    ray.Direction = direction;
    ray.Origin = origin;
    ray.TMax = RAY_TMAX;
    ray.TMin = RAY_TMIN;

    uint rayFlags = 0;
    TraceRay(g_RaytracingTlas, rayFlags, 0xFF, 0, 0, 0, ray, payload);

    return payload;
}

[shader("raygeneration")]
void RayGeneration()
{
    const uint2 screenCoords = DispatchRaysIndex().xy;

    if (any(screenCoords >= g_GlobalConstants.m_ScreenResolution.xy))
        return;

    const float sceneDepth = g_SceneDepth.Load(int3(screenCoords, 0)).r;

    if (sceneDepth <= 0) // Reverse-Z
        return;

    TranslucentRayPayload primaryRay = TracePrimaryRay(screenCoords, sceneDepth, 2);

    g_SceneColor[screenCoords].xyz += primaryRay.m_Radiance;
}

[shader("miss")]
void Miss(inout TranslucentRayPayload payload)
{
    payload.m_Radiance = SampleEnvironmentLighting(WorldRayDirection());
}

[shader("closesthit")]
void ClosestHit(inout TranslucentRayPayload payload, in BuiltInTriangleIntersectionAttributes attribs)
{
    const GeometryInfo geoInfo = g_GeometryInfo[InstanceIndex()];
    MeshVertex vertex = GetHitSurface(attribs, geoInfo);
    const Material material = g_MaterialTable[geoInfo.m_MaterialIndex];

    payload.m_Depth = max(0, (int)payload.m_Depth - 1);
    payload.m_Radiance = 0;

    // TODO: why is payload flags not updating?
    if (payload.IsPrimaryRay())
    {
        if (material.m_Opacity >= 0.99f)
            return;

        vertex.m_TexCoord *= 20;
        vertex.m_TexCoord += float2(0.2, -1) * g_GlobalConstants.m_Time.z;
        const ShadingSurface surface = GetShadingSurfaceFromHit(vertex, material, g_GlobalConstants.m_SamplerIndex_Linear_Wrap, INDIRECT_MIP_LEVEL);

        TranslucentRayPayload screenTracePayload;
        screenTracePayload.m_Depth = payload.m_Depth - 1;
        screenTracePayload.m_Radiance = 0;
        screenTracePayload.m_Flags = 0;

        RayDesc ray;
        ray.Direction = reflect(WorldRayDirection(), surface.m_Normal);
        ray.Origin = surface.m_Position;
        ray.TMax = RAY_TMAX;
        ray.TMin = RAY_TMIN;

        uint rayFlags = 0;
        TraceRay(g_RaytracingTlas, rayFlags, 0xFF, 0, 0, 0, ray, screenTracePayload);
        payload.m_Radiance = ComputeRadiance(surface, screenTracePayload.m_Radiance, ray.Direction, -WorldRayDirection());
    }
    else
    {
        const ShadingSurface surface = GetShadingSurfaceFromHit(vertex, material, g_GlobalConstants.m_SamplerIndex_Linear_Wrap, INDIRECT_MIP_LEVEL);
        payload.m_Radiance += BruteForcedIBL(surface, -WorldRayDirection());
    }

}

[shader("anyhit")]
void AnyHit(inout TranslucentRayPayload payload, in BuiltInTriangleIntersectionAttributes attribs)
{
    const GeometryInfo geoInfo = g_GeometryInfo[InstanceIndex()];
    const MeshVertex vertex = GetHitSurface(attribs, geoInfo);
    const Material material = g_MaterialTable[geoInfo.m_MaterialIndex];

    //TODO: Any hit needs to be updated to ignore translucent objects in ssr
    if (payload.IsPrimaryRay())
    {
        return;
    }

    // Ignore alpha masked materials (hack)
    float opacity = material.m_Opacity;
    if (material.m_BaseColorTextureIndex != 0)
    {
        sampler linearSampler = SamplerDescriptorHeap[g_GlobalConstants.m_SamplerIndex_Linear_Wrap];
        Texture2D<float4> albedoTex = ResourceDescriptorHeap[material.m_BaseColorTextureIndex];
        float4 gatherOpacity = albedoTex.GatherAlpha(linearSampler, vertex.m_TexCoord);
        opacity *= (gatherOpacity.x + gatherOpacity.y + gatherOpacity.z + gatherOpacity.w) / 4.0f;
    }

    if (opacity < 1.0)
        IgnoreHit();
}

#endif // __PATHTRACING_RGS_HLSL__
