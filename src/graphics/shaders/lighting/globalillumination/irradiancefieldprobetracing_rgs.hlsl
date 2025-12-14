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

#ifndef __IRRADIANCE_FIELD_PROBE_TRACING_HLSL__
#define __IRRADIANCE_FIELD_PROBE_TRACING_HLSL__

#define USE_IRRADIANCE_FIELD 1
#include "lighting/globalillumination/irradiancefield.hlsl"
#include "utils/raytracing.hlsl"

RayPayload TraceProbeRay(float3 origin, float3 direction)
{
    RayPayload payload;
    payload.m_IsShadowRay = false;

    // Only need 2, one for the probe ray, and one for direct light on hit. 
    // Infinite bounce is handled by reading back from irradiance cache with USE_IRRADIANCE_FIELD 1 
    payload.m_Depth = 2; 
    
    RayDesc ray;
    ray.Origin = origin;
    ray.Direction = normalize(direction);
    ray.TMax = RAY_TMAX;
    ray.TMin = RAY_TMIN;
    
    uint rayFlags = RAY_FLAG_FORCE_OPAQUE; // Don't do any-hit for GI rays for performance
    TraceRay(RTRaytracingTlas, rayFlags, 0xFF, 0, 0, 0, ray, payload);
    return payload;
}

void FillIrradianceBorder(uint2 sampleCoords)
{
    const uint2 localCoords = GetTileLocalCoords(sampleCoords, IrradianceFieldParams.m_IrradianceTileSize);
    const uint tileSize = IrradianceFieldParams.m_IrradianceTileSize;
    const uint2 tileOrigin = sampleCoords - localCoords;
    
    uint2 sourceLocal = clamp(localCoords, uint2(1, 1), uint2(tileSize - 2, tileSize - 2));

    if (localCoords.x == 0 || localCoords.x == tileSize - 1)
        sourceLocal.y = (tileSize - 1) - sourceLocal.y;
    
    if (localCoords.y == 0 || localCoords.y == tileSize - 1)
        sourceLocal.x = (tileSize - 1) - sourceLocal.x;
    
    const uint2 sourceCoords = tileOrigin + sourceLocal;
    RWIrradianceFieldIrradianceAtlas[sampleCoords] = RWIrradianceFieldIrradianceAtlas[sourceCoords];
}

[shader("raygeneration")]
void RayGeneration()
{
    const uint3 dispatchCoords = DispatchRaysIndex().xyz;
    
    const uint3 probeGridCoords = dispatchCoords;
    const uint probeIndex = probeGridCoords.x + 
                            probeGridCoords.y * IrradianceFieldParams.m_GridResolution.x +
                            probeGridCoords.z * IrradianceFieldParams.m_GridResolution.x * IrradianceFieldParams.m_GridResolution.y;
    
    float3 probeWorldPos = GetProbeWorldPosition(probeIndex);
    uint2 atlasOffset = GetAtlasOffset(probeIndex, IrradianceFieldParams.m_IrradianceTileSize);
    const uint tileSize = IrradianceFieldParams.m_IrradianceTileSize;
    
    const uint sqrtNumRays = 4;
    const uint numRays = sqrtNumRays * sqrtNumRays;
 
    // read previous frame's irradiance for this probe into local storage
    float3 previousIrradiance[64]; // Assuming max 8x8 interior = 64 texels
    uint texelIdx = 0;
    for (uint y = 1; y < tileSize - 1; y++)
    {
        for (uint x = 1; x < tileSize - 1; x++)
        {
            uint2 atlasCoords = atlasOffset + uint2(x, y);
            previousIrradiance[texelIdx++] = IrradianceFieldIrradianceAtlas[atlasCoords];
        }
    }
    
    // Initialize accumulation buffer to zero
    float3 accumulatedIrradiance[64];
    for (uint i = 0; i < 64; i++)
        accumulatedIrradiance[i] = 0;
    
    // Trace rays and accumulate
    for (uint rayIdx = 0; rayIdx < numRays; rayIdx++)
    {
        uint strataX = rayIdx % sqrtNumRays;
        uint strataY = rayIdx / sqrtNumRays;
        float2 jitter = CMJ_Sample2D(rayIdx, sqrtNumRays, sqrtNumRays, GlobalConstants.m_FrameNumber);
        float2 sampleUV = (float2(strataX, strataY) + jitter) / float(sqrtNumRays);
        float3 rayDirection = SampleDirectionSphere(sampleUV);
        float pdf = SampleDirectionSphere_Pdf();
        
        RayPayload payload = TraceProbeRay(probeWorldPos, rayDirection);
        
        // Splat to all texels with cosine weighting
        texelIdx = 0;
        for (uint y2 = 1; y2 < tileSize - 1; y2++)
        {
            for (uint x2 = 1; x2 < tileSize - 1; x2++)
            {
                uint2 localCoords = uint2(x2, y2);
                float3 texelDirection = TileTexelToDirection(localCoords, tileSize);
                float cosTheta = saturate(dot(rayDirection, texelDirection));
                accumulatedIrradiance[texelIdx] += payload.m_Radiance * cosTheta / (numRays * pdf);
                texelIdx++;
            }
        }
    }
    
    // Spatial filtering from neighboring probes
    const int3 neighborOffsets[6] = {
        int3(-1, 0, 0), int3(1, 0, 0),
        int3(0, -1, 0), int3(0, 1, 0),
        int3(0, 0, -1), int3(0, 0, 1)
    };
    
    float3 spatiallyFilteredIrradiance[64];
    texelIdx = 0;
    for (uint y3 = 1; y3 < tileSize - 1; y3++)
    {
        for (uint x3 = 1; x3 < tileSize - 1; x3++)
        {
            uint2 localCoords = uint2(x3, y3);
            float3 texelDirection = TileTexelToDirection(localCoords, tileSize);
            
            float3 centerIrradiance = accumulatedIrradiance[texelIdx];
            float3 neighborSum = 0;
            float totalWeight = 1.0; // Weight for center probe
            
            // Sample from immediate neighbors
            for (uint n = 0; n < 6; n++)
            {
                int3 neighborGridCoords = int3(probeGridCoords) + neighborOffsets[n];
                
                // Check bounds
                if (any(neighborGridCoords < 0) || 
                    any(neighborGridCoords >= int3(IrradianceFieldParams.m_GridResolution)))
                    continue;
                
                uint neighborProbeIndex = neighborGridCoords.x + 
                                         neighborGridCoords.y * IrradianceFieldParams.m_GridResolution.x +
                                         neighborGridCoords.z * IrradianceFieldParams.m_GridResolution.x * IrradianceFieldParams.m_GridResolution.y;
                
                uint2 neighborAtlasOffset = GetAtlasOffset(neighborProbeIndex, IrradianceFieldParams.m_IrradianceTileSize);
                uint2 neighborAtlasCoords = neighborAtlasOffset + localCoords;
                
                float3 neighborIrradiance = IrradianceFieldIrradianceAtlas[neighborAtlasCoords];
                
                // Weight by direction similarity (optional - helps preserve directional features)
                float3 toNeighbor = normalize(float3(neighborOffsets[n]));
                float directionWeight = saturate(dot(texelDirection, toNeighbor) * 0.5 + 0.5);
                
                neighborSum += neighborIrradiance * directionWeight;
                totalWeight += directionWeight;
            }
            
            spatiallyFilteredIrradiance[texelIdx] = (centerIrradiance + neighborSum) / totalWeight;
            texelIdx++;
        }
    }
    
    // Temporal blend with history
    texelIdx = 0;
    float temporalBlend = 0.98;
    for (uint y4 = 1; y4 < tileSize - 1; y4++)
    {
        for (uint x4 = 1; x4 < tileSize - 1; x4++)
        {
            float3 filteredIrradiance = spatiallyFilteredIrradiance[texelIdx];
            float3 blendedIrradiance = lerp(filteredIrradiance, previousIrradiance[texelIdx], temporalBlend);
            
            uint2 atlasCoords = atlasOffset + uint2(x4, y4);
            RWIrradianceFieldIrradianceAtlas[atlasCoords] = blendedIrradiance;
            texelIdx++;
        }
    }
    
    // Fill borders
    for (uint y5 = 0; y5 < tileSize; y5++)
    {
        for (uint x5 = 0; x5 < tileSize; x5++)
        {
            if (x5 > 0 && x5 < tileSize - 1 && y5 > 0 && y5 < tileSize - 1)
                continue;
            
            uint2 borderCoords = atlasOffset + uint2(x5, y5);
            FillIrradianceBorder(borderCoords);
        }
    }

}

#endif // __IRRADIANCE_FIELD_PROBE_TRACING_HLSL__
