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

#include "lighting/globalillumination/irradiancefield.hlsl"

RayPayload TraceProbeRay(float3 origin, float3 direction)
{
    RayPayload payload;
    payload.m_IsShadowRay = false;
    payload.m_Depth = MAX_DEPTH;
    
    RayDesc ray;
    ray.Origin = origin;
    ray.Direction = normalize(direction);
    ray.TMax = RAY_TMAX;
    ray.TMin = RAY_TMIN;
    
    uint rayFlags = RAY_FLAG_FORCE_OPAQUE; // Don't do any-hit for GI rays for performance
    TraceRay(RTRaytracingTlas, rayFlags, 0xFF, 0, 0, 0, ray, payload);
    return payload;
}

[shader("raygeneration")]
void RayGeneration()
{
    const uint2 sampleCoords = DispatchRaysIndex().xy;

    if (IsWithinIrradianceAtlasBounds(sampleCoords))
    {
        if (IsWithinIrradianceTileBounds(sampleCoords))
        {
            const uint probeIndex = GetTileIndex(sampleCoords, IrradianceFieldParams.m_IrradianceTileSize);
            const uint2 localCoords = GetTileLocalCoords(sampleCoords, IrradianceFieldParams.m_IrradianceTileSize);
            const float3 probeWorldPos = GetProbeWorldPosition(probeIndex);
            const float3 rayDirection = TileTexelToDirection(localCoords, IrradianceFieldParams.m_IrradianceTileSize - 1);

            RayPayload payload = TraceProbeRay(probeWorldPos, rayDirection);
            RWIrradianceFieldIrradianceAtlas[sampleCoords] = payload.m_Radiance;
        }
        else
        {
            // Zero out border texel 
            RWIrradianceFieldIrradianceAtlas[sampleCoords] = 0;
        }
    }
}

#endif // __IRRADIANCE_FIELD_PROBE_TRACING_HLSL__
