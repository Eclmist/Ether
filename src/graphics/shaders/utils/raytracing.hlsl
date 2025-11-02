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
#include "common/raytracingconstants.h"
#include "utils/encoding.hlsl"
#include "utils/shading.hlsl"
#include "lighting/brdf.hlsl"

#define SKYLIGHT_SCALE 1.0f

RaytracingAccelerationStructure g_RaytracingTlas    : register(t0);
StructuredBuffer<GeometryInfo> g_GeometryInfo       : register(t1);
StructuredBuffer<Material> g_MaterialTable          : register(t2);

float BarycentricLerp(in float v0, in float v1, in float v2, in float3 barycentrics)
{
    return v0 * barycentrics.x + v1 * barycentrics.y + v2 * barycentrics.z;
}

float2 BarycentricLerp(in float2 v0, in float2 v1, in float2 v2, in float3 barycentrics)
{
    return v0 * barycentrics.x + v1 * barycentrics.y + v2 * barycentrics.z;
}

float3 BarycentricLerp(in float3 v0, in float3 v1, in float3 v2, in float3 barycentrics)
{
    return v0 * barycentrics.x + v1 * barycentrics.y + v2 * barycentrics.z;
}

float4 BarycentricLerp(in float4 v0, in float4 v1, in float4 v2, in float3 barycentrics)
{
    return v0 * barycentrics.x + v1 * barycentrics.y + v2 * barycentrics.z;
}

MeshVertex BarycentricLerp(in MeshVertex v0, in MeshVertex v1, in MeshVertex v2, in float3 barycentrics)
{
    MeshVertex vtx;
    vtx.m_Position = BarycentricLerp(v0.m_Position, v1.m_Position, v2.m_Position, barycentrics);
    vtx.m_Normal = normalize(BarycentricLerp(v0.m_Normal, v1.m_Normal, v2.m_Normal, barycentrics));
    vtx.m_Tangent = normalize(BarycentricLerp(v0.m_Tangent, v1.m_Tangent, v2.m_Tangent, barycentrics));
    vtx.m_Color = BarycentricLerp(v0.m_Color, v1.m_Color, v2.m_Color, barycentrics);
    vtx.m_TexCoord = BarycentricLerp(v0.m_TexCoord, v1.m_TexCoord, v2.m_TexCoord, barycentrics);

    return vtx;
}

MeshVertex GetHitSurface(in BuiltInTriangleIntersectionAttributes attribs, in GeometryInfo geoInfo)
{
    float3 barycentrics;
    barycentrics.x = 1 - attribs.barycentrics.x - attribs.barycentrics.y;
    barycentrics.y = attribs.barycentrics.x;
    barycentrics.z = attribs.barycentrics.y;

    StructuredBuffer<MeshVertex> vtxBuffer = ResourceDescriptorHeap[geoInfo.m_VBDescriptorIndex];
    Buffer<uint> idxBuffer = ResourceDescriptorHeap[geoInfo.m_IBDescriptorIndex];

    const uint primIdx = PrimitiveIndex();
    const uint idx0 = idxBuffer[primIdx * 3 + 0];
    const uint idx1 = idxBuffer[primIdx * 3 + 1];
    const uint idx2 = idxBuffer[primIdx * 3 + 2];

    const MeshVertex v0 = vtxBuffer[idx0];
    const MeshVertex v1 = vtxBuffer[idx1];
    const MeshVertex v2 = vtxBuffer[idx2];

    return BarycentricLerp(v0, v1, v2, barycentrics);
}

float3 ComputeRadiance(ShadingSurface surface, float3 Li, float3 wi, float3 wo)
{
    wi = normalize(wi);
    wo = normalize(wo);
    const float3 f = BRDF_UE4(wi, wo, surface.m_Normal, surface.m_Albedo, surface.m_Roughness, surface.m_Metalness);
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

void SampleDirectionUniform(ShadingSurface surface, float seed, out float3 wi, out float pdf)
{
    const uint2 sampleCoords = DispatchRaysIndex().xy;
    const uint2 bufferSize = DispatchRaysDimensions().xy;
    const uint sampleIdx = sampleCoords.y * bufferSize.x + sampleCoords.x;
    const float2 rand2D = CMJ_Sample2D(sampleIdx, 1024, 1024, seed);
    wi = TangentToWorld(SampleDirectionHemisphere(rand2D), surface.m_Normal);
    pdf = SampleDirectionHemisphere_Pdf();

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

float3 SampleEnvironmentLighting(float3 wi)
{
    return SampleEnvironmentLighting(wi, 4);
}

RayPayload TraceShadowRay(ShadingSurface surface, float3 direction)
{
    direction = normalize(direction);
    
    RayPayload payload;
    payload.m_IsShadowRay = true;
    payload.m_Depth = 1;
    payload.m_Radiance = 0;

    RayDesc ray;
    ray.Direction = direction;
    ray.Origin = surface.m_Position + surface.m_Normal * 0.01;
    ray.TMax = RAY_TMAX;
    ray.TMin = RAY_TMIN;

    uint rayFlags = RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH;
    TraceRay(g_RaytracingTlas, rayFlags, 0xFF, 0, 0, 0, ray, payload);

    return payload;
}

RayPayload TraceShadingRay(ShadingSurface surface, float3 direction, uint depth)
{
    direction = normalize(direction);
    
    RayPayload payload;
    payload.m_IsShadowRay = false;
    payload.m_Depth = depth;

    RayDesc ray;
    ray.Origin = surface.m_Position + surface.m_Normal * 0.01;
    ray.Direction = direction;
    ray.TMax = RAY_TMAX;
    ray.TMin = RAY_TMIN;

    uint rayFlags = RAY_FLAG_FORCE_OPAQUE; // Don't do any-hit for GI rays for performance reasons
    TraceRay(g_RaytracingTlas, rayFlags, 0xFF, 0, 0, 0, ray, payload);

    return payload;
}

[shader("miss")]
void Miss(inout RayPayload payload)
{
    payload.m_Hit = false;
    payload.m_HitPosition = WorldRayOrigin() + WorldRayDirection() * 9999.0f;
    payload.m_Radiance = 0;

    if (payload.m_IsShadowRay)
    {
        // Sample sun color
        const float lerpFactor = saturate(dot(g_GlobalConstants.m_SunDirection.xyz, float3(0, 1, 0)));
        payload.m_Radiance = lerp(0.0f, g_GlobalConstants.m_SunColor.xyz, lerpFactor);
    }
    else
    {
        // Sample environment color
        payload.m_Radiance = SampleEnvironmentLighting(WorldRayDirection()) * SKYLIGHT_SCALE;
    }
}

[shader("closesthit")]
void ClosestHit(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attribs)
{
    const GeometryInfo geoInfo = g_GeometryInfo[InstanceIndex()];
    const MeshVertex vertex = GetHitSurface(attribs, geoInfo);
    const Material material = g_MaterialTable[geoInfo.m_MaterialIndex];
    const ShadingSurface surface = GetShadingSurfaceFromHit(vertex, material, g_GlobalConstants.m_SamplerIndex_Linear_Wrap, INDIRECT_MIP_LEVEL);
    const float3 viewDir = normalize(g_GlobalConstants.m_CameraPosition.xyz - surface.m_Position);

    payload.m_Hit = true;
    payload.m_HitPosition = surface.m_Position;
    payload.m_HitNormal = surface.m_Normal;
    payload.m_Depth = max(0, (int)payload.m_Depth - 1);
    payload.m_Radiance = 0;

    if (payload.m_Depth <= 0)
        return;

    if (payload.m_IsShadowRay)
        return;

    float3 direct = 0.0f;
    float3 indirect = 0.0f;

    {   // Direct lighting
        const RayPayload shadowRay = TraceShadowRay(surface, g_GlobalConstants.m_SunDirection.xyz);
        direct = ComputeRadiance(surface, shadowRay.m_Radiance, g_GlobalConstants.m_SunDirection.xyz, -WorldRayDirection());
    }

    {   // Indirect lighting
        float3 wi;
        float pdf;

#if USE_IMPORTANCE_SAMPLING
        SampleDirectionBrdf(surface, g_GlobalConstants.m_FrameNumber, viewDir, wi, pdf);
#else
        SampleDirectionUniform(surface, g_GlobalConstants.m_FrameNumber, wi, pdf);
#endif

        if (pdf > 0.1f)
        {
            const RayPayload indirectRay = TraceShadingRay(surface, wi, payload.m_Depth);
            indirect = ComputeRadiance(surface, indirectRay.m_Radiance, wi, -WorldRayDirection()) / pdf;
        }
        
    }

    payload.m_Radiance = surface.m_Emission + direct + indirect;
}

[shader("anyhit")]
void AnyHit(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attribs)
{
    const GeometryInfo geoInfo = g_GeometryInfo[InstanceIndex()];
    const MeshVertex vertex = GetHitSurface(attribs, geoInfo);
    const Material material = g_MaterialTable[geoInfo.m_MaterialIndex];

    // Early out if not masked
    //if (!mat.IsMasked()) (TODO)
    //    return;

    float opacity = material.m_BaseColor.a; // ignore vertex color alpha for now (TODO)
    if (material.m_AlbedoTextureIndex != 0)
    {
        sampler linearSampler = SamplerDescriptorHeap[g_GlobalConstants.m_SamplerIndex_Linear_Wrap];
        Texture2D<float4> albedoTex = ResourceDescriptorHeap[material.m_AlbedoTextureIndex];
        float4 gatherOpacity = albedoTex.GatherAlpha(linearSampler, vertex.m_TexCoord);
        opacity *= (gatherOpacity.x + gatherOpacity.y + gatherOpacity.z + gatherOpacity.w) / 4.0f;
    }

    if (opacity < 1.0f)
    {
        IgnoreHit();
    }
}

