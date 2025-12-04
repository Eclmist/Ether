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

#ifndef __RAYTRACING_HLSL__
#define __RAYTRACING_HLSL__
#define SCALE 65536.0f
#include "common/globalconstants.h"
#include "common/raytracingconstants.h"
#include "utils/encoding.hlsl"
#include "utils/shading.hlsl"
#include "utils/brdf.hlsl"

// Spatial Hash Prototype
RWStructuredBuffer<uint> RWSpatialHash                      : register(u4);
RWStructuredBuffer<uint> RWSpatialHashAge                   : register(u5);
RWStructuredBuffer<SpatialHashPayload> RWSpatialHashPayload : register(u6);

#define SEARCH_COUNT 10
#define HASH_NORMAL 1
#define MAX_HASH_AGE 30

//https://www.shadertoy.com/view/XlGcRh
uint pcg(uint v)
{
    uint state = v * 747796405u + 2891336453u;
    uint word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
    return (word >> 22u) ^ word;
}

//https://www.shadertoy.com/view/XlGcRh
uint xxhash32(uint p)
{
    const uint PRIME32_2 = 2246822519U, PRIME32_3 = 3266489917U;
    const uint PRIME32_4 = 668265263U, PRIME32_5 = 374761393U;
    uint h32 = p + PRIME32_5;
    h32 = PRIME32_4 * ((h32 << 17) | (h32 >> (32 - 17)));
    h32 = PRIME32_2 * (h32 ^ (h32 >> 15));
    h32 = PRIME32_3 * (h32 ^ (h32 >> 13));
    return h32 ^ (h32 >> 16);
}

uint SpatialHash_Lookup(float3 position, float3 normal)
{
    float cellSize = GlobalConstants.m_SpatialHashCellSize;
    const uint HashmapSize = GlobalConstants.m_SpatialHashSize;

    // Inputs to hashing
    int3 p = floor(position / cellSize);
    int3 n = floor(normal * 3.0);

    #if !HASH_NORMAL
        n = 0;
    #endif
     
    cellSize *= 10000; // cellSize can be small and lead to more conflicts, multiply to increase range
     
    uint hashKey = pcg(cellSize + pcg(p.x + pcg(p.y + pcg(p.z + pcg(n.x + pcg(n.y + pcg(n.z)))))));
        
    uint cellIndex = hashKey % HashmapSize;
           
    uint checksum = xxhash32(cellSize + xxhash32(p.x + xxhash32(p.y + xxhash32(p.z + xxhash32(n.x + xxhash32(n.y + xxhash32(n.z)))))));
    checksum = max(checksum, 1); // 0 is reserved for available cells
         
    // Update data structure
    for (uint i = 0; i < SEARCH_COUNT; i++)
    {                
        if (RWSpatialHash[cellIndex] == checksum)
            return cellIndex;
                          
        cellIndex++;
 
        if(cellIndex >= HashmapSize)
            break;
    }
 
    return  0xFFFFFFFFu; // out of memory 
}

//Adapted from https://gboisse.github.io/posts/this-is-us/
uint SpatialHash_FindOrInsert(float3 position, float3 normal)
{
    float cellSize = GlobalConstants.m_SpatialHashCellSize;
    const uint HashmapSize = GlobalConstants.m_SpatialHashSize;
    uint FrameIndex = GlobalConstants.m_FrameNumber;

    // Inputs to hashing
    int3 p = floor(position / cellSize);
    int3 n = floor(normal * 3.0);

    #if !HASH_NORMAL
        n = 0;
    #endif
     
    cellSize *= 10000; // cellSize can be small and lead to more conflicts, multiply to increase range
     
    uint hashKey = pcg(cellSize + pcg(p.x + pcg(p.y + pcg(p.z + pcg(n.x + pcg(n.y + pcg(n.z)))))));
        
    uint cellIndex = hashKey % HashmapSize;
           
    uint checksum = xxhash32(cellSize + xxhash32(p.x + xxhash32(p.y + xxhash32(p.z + xxhash32(n.x + xxhash32(n.y + xxhash32(n.z)))))));
    checksum = max(checksum, 1); // 0 is reserved for available cells
         
	// Update data structure
	for (uint i = 0; i < SEARCH_COUNT; i++)
	{                
		uint cmp;        
		InterlockedCompareExchange(RWSpatialHash[cellIndex], 0, checksum, cmp);
		 
		uint originalTime;
        bool isCorrectCell = false;
		if (cmp == 0 || cmp == checksum)
		{
			//InterlockedExchange(RWSpatialHashAge[cellIndex], FrameIndex, originalTime);
            isCorrectCell = true;
		}
		 
		originalTime = RWSpatialHashAge[cellIndex];

        if (isCorrectCell && FrameIndex - originalTime < MAX_HASH_AGE)
			return cellIndex; 

		if (FrameIndex - originalTime >= MAX_HASH_AGE)
		{
            SpatialHashPayload emptyPayload;
            emptyPayload.m_Radiance = 0;
            emptyPayload.m_NumSamples = 0;
            RWSpatialHashPayload[cellIndex] = emptyPayload;

            uint original;
			InterlockedExchange(RWSpatialHash[cellIndex], checksum, original);
			InterlockedExchange(RWSpatialHashAge[cellIndex], FrameIndex, originalTime);
			
			return cellIndex;
		}
		 
		cellIndex++;
		if (cellIndex >= HashmapSize)
			break;       
	} 

    return  0xFFFFFFFFu; // out of memory 
}

float3 ComputeRadiance(ShadingSurface surface, float3 Li, float3 wi, float3 wo)
{
    wi = normalize(wi);
    wo = normalize(wo);
    const float3 f = BRDF_UE4(wi, wo, surface.m_Normal, surface.m_BaseColor, surface.m_Roughness, surface.m_Metalness);
    const float cosTheta = saturate(dot(wi, surface.m_Normal));
    return f * Li * cosTheta;
}

float3 ComputeIrradiance(ShadingSurface surface, float3 Li, float3 wi)
{
    wi = normalize(wi);
    const float cosTheta = saturate(dot(wi, surface.m_Normal));
    return Li * cosTheta;
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
    sampler linearSampler = SamplerDescriptorHeap[GlobalConstants.m_SamplerIndex_Linear_Wrap];
    Texture2D<float4> hdriTexture = ResourceDescriptorHeap[GlobalConstants.m_HdriTextureIndex];
    const float exposure = GlobalConstants.m_SkyIntensity;

    const float2 hdriUv = SampleSphericalMap(wi);
    const float4 hdri = hdriTexture.SampleLevel(linearSampler, hdriUv, mipLevel);
    const float sunsetFactor = saturate(asin(dot(GlobalConstants.m_SunDirection.xyz, float3(0, 1, 0))));
    const float sunlightFactor = 1 - saturate(asin(dot(GlobalConstants.m_SunDirection.xyz, float3(0, -1, 0))));

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
    ray.Origin = surface.m_Position + surface.m_Normal * RAY_TMIN;
    ray.TMax = RAY_TMAX;
    ray.TMin = RAY_TMIN;

    uint rayFlags = RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH;
    TraceRay(RTRaytracingTlas, rayFlags, 0xFF, 0, 0, 0, ray, payload);

    return payload;
}

RayPayload TraceShadingRay(ShadingSurface surface, float3 direction, uint depth)
{
    direction = normalize(direction);
    
    RayPayload payload;
    payload.m_IsShadowRay = false;
    payload.m_Depth = depth;
    payload.m_Radiance = 0;

    RayDesc ray;
    ray.Origin = surface.m_Position + surface.m_Normal * RAY_TMIN;
    ray.Direction = direction;
    ray.TMax = RAY_TMAX;
    ray.TMin = RAY_TMIN;

    uint rayFlags = RAY_FLAG_FORCE_OPAQUE; // Don't do any-hit for GI rays for performance reasons
    TraceRay(RTRaytracingTlas, rayFlags, 0xFF, 0, 0, 0, ray, payload);

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
        const float lerpFactor = saturate(dot(GlobalConstants.m_SunDirection.xyz, float3(0, 1, 0)));
        payload.m_Radiance = lerp(0.0f, GlobalConstants.m_SunColor.xyz, lerpFactor);
    }
    else
    {
        // Sample environment color
        payload.m_Radiance = SampleEnvironmentLighting(WorldRayDirection());
    }
}

[shader("closesthit")]
void ClosestHit(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attribs)
{
    if (payload.m_IsShadowRay)
    {
        payload.m_Hit = true;
        return;
    }

    const GeometryInfo geoInfo = RTGeometryInfo[InstanceIndex()];
    const MeshVertex vertex = GetHitSurface(attribs, geoInfo);
    const Material material = MaterialTable[geoInfo.m_MaterialIndex];
    const ShadingSurface surface = GetShadingSurfaceFromHit(vertex, material, GlobalConstants.m_SamplerIndex_Linear_Wrap, INDIRECT_MIP_LEVEL);

    payload.m_Hit = true;
    payload.m_HitPosition = surface.m_Position;
    payload.m_HitNormal = surface.m_Normal;
    payload.m_Depth = max(0, (int)payload.m_Depth - 1);
    payload.m_Radiance = surface.m_Emission;

    if (payload.m_Depth <= 0)
        return;

    // Direct lighting
    {
        const RayPayload shadowRay = TraceShadowRay(surface, GlobalConstants.m_SunDirection.xyz);
        payload.m_Radiance += ComputeRadiance(surface, shadowRay.m_Radiance, GlobalConstants.m_SunDirection.xyz, -WorldRayDirection());

    }


#if USE_IRRADIANCE_CACHE
    uint cellIndex = SpatialHash_Lookup(surface.m_Position, surface.m_Normal);
    float3 irradianceCache = 0;
    float numSamples = 0;

    if (cellIndex != 0xFFFFFFFFu)
    {
        numSamples = RWSpatialHashPayload[cellIndex].m_NumSamples;
        irradianceCache.x = RWSpatialHashPayload[cellIndex].m_Radiance.x / numSamples;
        irradianceCache.y = RWSpatialHashPayload[cellIndex].m_Radiance.y / numSamples;
        irradianceCache.z = RWSpatialHashPayload[cellIndex].m_Radiance.z / numSamples;
        payload.m_Radiance += irradianceCache * surface.m_BaseColor / Pi;
    }
#endif

    // Indirect lighting
    {
        float3 wi;
        float pdf;

#if USE_IMPORTANCE_SAMPLING
        SampleDirectionBrdf(surface, payload.m_Depth, -WorldRayDirection(), wi, pdf);
#else
        SampleDirectionUniform(surface, payload.m_Depth, wi, pdf);
#endif

        if (pdf > 0.01f)
        {
            const RayPayload indirectRay = TraceShadingRay(surface, wi, payload.m_Depth);
            payload.m_Radiance += ComputeRadiance(surface, indirectRay.m_Radiance, wi, -WorldRayDirection()) / pdf;
            irradiance += ComputeIrradiance(surface, indirectRay.m_Radiance, wi) / pdf;
        }
    }

#if USE_IRRADIANCE_CACHE
    float3 positionJitter = 0;//Random3D(DispatchRaysIndex().xy, GlobalConstants.m_FrameNumber) - 0.5f;
    cellIndex = SpatialHash_FindOrInsert(surface.m_Position + positionJitter, surface.m_Normal);

    // Spatial hash experiment: increment irradiance
    if (cellIndex != 0xFFFFFFFFu)
    {
        InterlockedAdd(RWSpatialHashPayload[cellIndex].m_Radiance.x, (uint)(irradiance.x));
        InterlockedAdd(RWSpatialHashPayload[cellIndex].m_Radiance.y, (uint)(irradiance.y));
        InterlockedAdd(RWSpatialHashPayload[cellIndex].m_Radiance.z, (uint)(irradiance.z));
        InterlockedAdd(RWSpatialHashPayload[cellIndex].m_NumSamples, 1);
    }
#endif
}

[shader("anyhit")]
void AnyHit(inout RayPayload payload, in BuiltInTriangleIntersectionAttributes attribs)
{
    const GeometryInfo geoInfo = RTGeometryInfo[InstanceIndex()];
    const MeshVertex vertex = GetHitSurface(attribs, geoInfo);
    const Material material = MaterialTable[geoInfo.m_MaterialIndex];

    // Early out if not masked
    //if (!mat.IsMasked()) (TODO)
    //    return;

    float opacity = material.m_Opacity;
    if (material.m_BaseColorTextureIndex != 0)
    {
        sampler linearSampler = SamplerDescriptorHeap[GlobalConstants.m_SamplerIndex_Linear_Wrap];
        Texture2D<float4> albedoTex = ResourceDescriptorHeap[material.m_BaseColorTextureIndex];
        float4 gatherOpacity = albedoTex.GatherAlpha(linearSampler, vertex.m_TexCoord);
        opacity *= (gatherOpacity.x + gatherOpacity.y + gatherOpacity.z + gatherOpacity.w) / 4.0f;
    }

    if (opacity < 0.5f)
    {
        IgnoreHit();
    }
}

#endif // __RAYTRACING_HLSL__
