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

[shader("raygeneration")]
void RayGeneration()
{
    const uint2 sampleCoords = DispatchRaysIndex().xy;

    if (IsWithinIrradianceAtlasBounds(sampleCoords))
    {
        if (IsWithinIrradianceProbeBounds(sampleCoords))
        {
            uint probeIndex = GetProbeIndex(sampleCoords, IrradianceFieldParams.m_IrradianceTileSize);
            uint2 localCoords = GetProbeLocalCoords(sampleCoords, IrradianceFieldParams.m_IrradianceTileSize);
            float3 rayDirection = ProbeTexelCoordsToDirection(localCoords, IrradianceFieldParams.m_IrradianceTileSize - 1);
            RWIrradianceFieldIrradianceAtlas[sampleCoords] = rayDirection;
        }
        else
        {
            // Zero out border texel 
            RWIrradianceFieldIrradianceAtlas[sampleCoords] = 0;
        }
    }
}

#endif // __IRRADIANCE_FIELD_PROBE_TRACING_HLSL__
