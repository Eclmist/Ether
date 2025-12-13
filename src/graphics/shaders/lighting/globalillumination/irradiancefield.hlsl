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

#ifndef __IRRADIANCE_FIELD_HLSL__
#define __IRRADIANCE_FIELD_HLSL__

#include "common/irradiancefieldparams.h"
#include "utils/helpers.hlsl"
#include "utils/sampling.hlsl"
#include "utils/encoding.hlsl"

ConstantBuffer<IrradianceFieldParams> IrradianceFieldParams : register(b1);

Texture2D<float3> IrradianceFieldIrradianceAtlas            : register(t3);
Texture2D<float2> IrradianceFieldDepthAtlas                 : register(t4);

RWTexture2D<float3> RWIrradianceFieldIrradianceAtlas        : register(u0);
RWTexture2D<float2> RWIrradianceFieldDepthAtlas             : register(u1);

uint GetTotalProbeCount()
{
    return IrradianceFieldParams.m_GridResolution.x * IrradianceFieldParams.m_GridResolution.y * IrradianceFieldParams.m_GridResolution.z;
}

uint GetNumProbesPerRow()
{
    return IrradianceFieldParams.m_GridResolution.x * IrradianceFieldParams.m_GridResolution.y;
}

uint2 GetTileLocalCoords(uint2 dispatchCoords, uint tileSize)
{
    return uint2(dispatchCoords.x % tileSize, dispatchCoords.y % tileSize);
}

bool IsWithinIrradianceAtlasBounds(uint2 dispatchCoords)
{
    return dispatchCoords.x < IrradianceFieldParams.m_IrradianceAtlasResolution.x &&
           dispatchCoords.y < IrradianceFieldParams.m_IrradianceAtlasResolution.y;
}

bool IsWithinDepthAtlasBounds(uint2 dispatchCoords)
{
    return dispatchCoords.x < IrradianceFieldParams.m_DepthAtlasResolution.x &&
           dispatchCoords.y < IrradianceFieldParams.m_DepthAtlasResolution.y;
}

bool IsWithinIrradianceTileBounds(uint2 dispatchCoords)
{
    uint2 localCoords = GetTileLocalCoords(dispatchCoords, IrradianceFieldParams.m_IrradianceTileSize);
    
    // Skip all borders: left (0), top (0), right (last), bottom (last)
    if (localCoords.x == 0 || localCoords.y == 0 ||
        localCoords.x == IrradianceFieldParams.m_IrradianceTileSize - 1 ||
        localCoords.y == IrradianceFieldParams.m_IrradianceTileSize - 1)
        return false;

    return true;
}

uint GetTileIndex(uint2 dispatchCoords, uint tileSize)
{
    return (dispatchCoords.x / tileSize) + (dispatchCoords.y / tileSize) * GetNumProbesPerRow();
}

float3 TileTexelToDirection(uint2 localCoords, uint tileSize)
{
    // Subtract 1 to skip left/top border, then map interior texels to [0,1]
    return OctahedralDecode((float2(localCoords - 1) + 0.5) / float(tileSize - 2 /* 2px of borders in total */));
}

uint3 ProbeIndexToGridCoords(uint probeIndex)
{
    uint3 gridCoords;
    gridCoords.x = probeIndex % IrradianceFieldParams.m_GridResolution.x;
    gridCoords.y = (probeIndex / IrradianceFieldParams.m_GridResolution.x) % IrradianceFieldParams.m_GridResolution.y;
    gridCoords.z = probeIndex / (IrradianceFieldParams.m_GridResolution.x * IrradianceFieldParams.m_GridResolution.y);
    return gridCoords;
}

float3 GetProbeWorldPosition(uint probeIndex)
{
    uint3 gridCoords = ProbeIndexToGridCoords(probeIndex);
    float3 gridPos = float3(gridCoords);
    float3 gridExtent = float3(IrradianceFieldParams.m_GridResolution - 1) * 0.5;
    return IrradianceFieldParams.m_GridOrigin + (gridPos - gridExtent) * IrradianceFieldParams.m_GridSpacing;
}

uint2 GetAtlasOffset(uint probeIndex, uint tileSize)
{
    uint probesPerRow = GetNumProbesPerRow();
    uint probeX = probeIndex % probesPerRow;
    uint probeY = probeIndex / probesPerRow;
    return uint2(probeX * tileSize, probeY * tileSize);
}

float3 SampleProbeIrradiance(uint probeIndex, float3 worldDir)
{
    sampler linearSampler = SamplerDescriptorHeap[GlobalConstants.m_SamplerIndex_Linear_Clamp];
    
    float2 octCoords = OctahedralEncode(normalize(worldDir));
    uint2 atlasOffset = GetAtlasOffset(probeIndex, IrradianceFieldParams.m_IrradianceTileSize);
    uint tileSize = IrradianceFieldParams.m_IrradianceTileSize;
    
    float2 pixelCoord = (octCoords * (tileSize - 2)) + 1;

    float2 atlasUV = (float2(atlasOffset) + pixelCoord) / float2(IrradianceFieldParams.m_IrradianceAtlasResolution);

    return IrradianceFieldIrradianceAtlas.SampleLevel(linearSampler, atlasUV, 0).rgb;
}

float3 SampleIrradianceField(float3 position, float3 normal)
{
    float3 gridExtent = float3(IrradianceFieldParams.m_GridResolution - 1) * 0.5;
    float3 relativePos = (position - IrradianceFieldParams.m_GridOrigin) / IrradianceFieldParams.m_GridSpacing;
    float3 gridPos = relativePos + gridExtent;
    
    int3 baseGridCoords = int3(floor(gridPos));
    
    float3 alpha = frac(gridPos);
    
    int3 gridCoords000 = clamp(baseGridCoords + int3(0, 0, 0), int3(0, 0, 0), int3(IrradianceFieldParams.m_GridResolution) - 1);
    int3 gridCoords001 = clamp(baseGridCoords + int3(0, 0, 1), int3(0, 0, 0), int3(IrradianceFieldParams.m_GridResolution) - 1);
    int3 gridCoords010 = clamp(baseGridCoords + int3(0, 1, 0), int3(0, 0, 0), int3(IrradianceFieldParams.m_GridResolution) - 1);
    int3 gridCoords011 = clamp(baseGridCoords + int3(0, 1, 1), int3(0, 0, 0), int3(IrradianceFieldParams.m_GridResolution) - 1);
    int3 gridCoords100 = clamp(baseGridCoords + int3(1, 0, 0), int3(0, 0, 0), int3(IrradianceFieldParams.m_GridResolution) - 1);
    int3 gridCoords101 = clamp(baseGridCoords + int3(1, 0, 1), int3(0, 0, 0), int3(IrradianceFieldParams.m_GridResolution) - 1);
    int3 gridCoords110 = clamp(baseGridCoords + int3(1, 1, 0), int3(0, 0, 0), int3(IrradianceFieldParams.m_GridResolution) - 1);
    int3 gridCoords111 = clamp(baseGridCoords + int3(1, 1, 1), int3(0, 0, 0), int3(IrradianceFieldParams.m_GridResolution) - 1);
    
    uint probeIndex000 = gridCoords000.x + gridCoords000.y * IrradianceFieldParams.m_GridResolution.x + gridCoords000.z * IrradianceFieldParams.m_GridResolution.x * IrradianceFieldParams.m_GridResolution.y;
    uint probeIndex001 = gridCoords001.x + gridCoords001.y * IrradianceFieldParams.m_GridResolution.x + gridCoords001.z * IrradianceFieldParams.m_GridResolution.x * IrradianceFieldParams.m_GridResolution.y;
    uint probeIndex010 = gridCoords010.x + gridCoords010.y * IrradianceFieldParams.m_GridResolution.x + gridCoords010.z * IrradianceFieldParams.m_GridResolution.x * IrradianceFieldParams.m_GridResolution.y;
    uint probeIndex011 = gridCoords011.x + gridCoords011.y * IrradianceFieldParams.m_GridResolution.x + gridCoords011.z * IrradianceFieldParams.m_GridResolution.x * IrradianceFieldParams.m_GridResolution.y;
    uint probeIndex100 = gridCoords100.x + gridCoords100.y * IrradianceFieldParams.m_GridResolution.x + gridCoords100.z * IrradianceFieldParams.m_GridResolution.x * IrradianceFieldParams.m_GridResolution.y;
    uint probeIndex101 = gridCoords101.x + gridCoords101.y * IrradianceFieldParams.m_GridResolution.x + gridCoords101.z * IrradianceFieldParams.m_GridResolution.x * IrradianceFieldParams.m_GridResolution.y;
    uint probeIndex110 = gridCoords110.x + gridCoords110.y * IrradianceFieldParams.m_GridResolution.x + gridCoords110.z * IrradianceFieldParams.m_GridResolution.x * IrradianceFieldParams.m_GridResolution.y;
    uint probeIndex111 = gridCoords111.x + gridCoords111.y * IrradianceFieldParams.m_GridResolution.x + gridCoords111.z * IrradianceFieldParams.m_GridResolution.x * IrradianceFieldParams.m_GridResolution.y;
    
    float weight000 = (1.0 - alpha.x) * (1.0 - alpha.y) * (1.0 - alpha.z);
    float weight001 = (1.0 - alpha.x) * (1.0 - alpha.y) * alpha.z;
    float weight010 = (1.0 - alpha.x) * alpha.y * (1.0 - alpha.z);
    float weight011 = (1.0 - alpha.x) * alpha.y * alpha.z;
    float weight100 = alpha.x * (1.0 - alpha.y) * (1.0 - alpha.z);
    float weight101 = alpha.x * (1.0 - alpha.y) * alpha.z;
    float weight110 = alpha.x * alpha.y * (1.0 - alpha.z);
    float weight111 = alpha.x * alpha.y * alpha.z;
    
    return SampleProbeIrradiance(probeIndex000, normal) * weight000 +
           SampleProbeIrradiance(probeIndex001, normal) * weight001 +
           SampleProbeIrradiance(probeIndex010, normal) * weight010 +
           SampleProbeIrradiance(probeIndex011, normal) * weight011 +
           SampleProbeIrradiance(probeIndex100, normal) * weight100 +
           SampleProbeIrradiance(probeIndex101, normal) * weight101 +
           SampleProbeIrradiance(probeIndex110, normal) * weight110 +
           SampleProbeIrradiance(probeIndex111, normal) * weight111;
}

#endif // __IRRADIANCE_FIELD_HLSL__
