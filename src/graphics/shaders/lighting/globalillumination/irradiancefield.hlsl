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
#include "utils/raytracing.hlsl"

ConstantBuffer<IrradianceFieldParams> IrradianceFieldParams : register(b1);

Texture2D<float3> IrradianceFieldIrradianceAtlas            : register(t0);
Texture2D<float2> IrradianceFieldDepthAtlas                 : register(t1);

RWTexture2D<float3> RWIrradianceFieldIrradianceAtlas        : register(u0);
RWTexture2D<float2> RWIrradianceFieldDepthAtlas             : register(u1);

uint GetTotalProbeCount()
{
    return IrradianceFieldParams.m_GridResolution.x * IrradianceFieldParams.m_GridResolution.y * IrradianceFieldParams.m_GridResolution.z;
}

uint GetIrradianceProbesPerRow()
{
    return IrradianceFieldParams.m_IrradianceAtlasResolution.x / IrradianceFieldParams.m_IrradianceTileSize;
}

uint GetDepthProbesPerRow()
{
    return IrradianceFieldParams.m_DepthAtlasResolution.x / IrradianceFieldParams.m_DepthTileSize;
}

uint2 GetProbeLocalCoords(uint2 dispatchCoords, uint tileSize)
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

bool IsWithinIrradianceProbeBounds(uint2 dispatchCoords)
{
    uint2 localCoords = GetProbeLocalCoords(dispatchCoords, IrradianceFieldParams.m_IrradianceTileSize);
    
    if (any(localCoords == IrradianceFieldParams.m_IrradianceTileSize - 1))
        return false;

    return true;
}

uint GetProbeIndex(uint2 dispatchCoords, uint tileSize)
{
    uint probesPerRow = GetDepthProbesPerRow();
    return (dispatchCoords.x / IrradianceFieldParams.m_DepthTileSize) + 
           (dispatchCoords.y / IrradianceFieldParams.m_DepthTileSize) * probesPerRow;
}

float3 ProbeTexelCoordsToDirection(uint2 localCoords, uint tileSize)
{
    return OctahedralDecode((float2(localCoords) + 0.5) / float(tileSize));
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
    uint probesPerRow = GetIrradianceProbesPerRow();
    uint probeX = probeIndex % probesPerRow;
    uint probeY = probeIndex / probesPerRow;
    return uint2(probeX * tileSize, probeY * tileSize);
}

float3 SampleProbeIrradiance(uint probeIndex, float3 worldDir)
{
    sampler linearSampler = SamplerDescriptorHeap[GlobalConstants.m_SamplerIndex_Linear_Clamp];

    const float2 octCoords = OctahedralEncode(normalize(worldDir));
    const uint2 atlasOffset = GetAtlasOffset(probeIndex, IrradianceFieldParams.m_IrradianceTileSize);
 
    const uint probeResolution = IrradianceFieldParams.m_IrradianceTileSize - 2; // 1px border on left + border on right
    const float2 texelCoords = octCoords * probeResolution;

    const float2 atlasUV = (float2(atlasOffset) + texelCoords + 0.5) / float2(IrradianceFieldParams.m_IrradianceAtlasResolution);
    return IrradianceFieldIrradianceAtlas.SampleLevel(linearSampler, atlasUV, 0).rgb;
}

#endif // __IRRADIANCE_FIELD_HLSL__
