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

#pragma once

#include "hlsltranslation.h"

ETH_BEGIN_SHADER_NAMESPACE

#define INDIRECT_MIP_LEVEL  8
#define MAX_DEPTH           2

// TODO: implement various biases (pullback, surface, etc.)
#define RAY_TMAX            128
#define RAY_TMIN            0.1

#define USE_IMPORTANCE_SAMPLING 1

struct GeometryInfo
{
    uint32_t m_VBDescriptorIndex;
    uint32_t m_IBDescriptorIndex;
    uint32_t m_MaterialIndex;
    uint32_t m_PadTo16Bytes;
};

struct RayPayload
{
    // TODO: Flag these bools and update payload size in the producers
    bool m_Hit;
    bool m_IsShadowRay;
    uint32_t m_Depth;
    ethVector3 m_Radiance;
    ethVector3 m_HitPosition;
    ethVector3 m_HitNormal;
};

struct TranslucentRayPayload
{
    ethVector3 m_Radiance;
    uint32_t m_Depth;
    uint32_t m_Flags;

    bool IsHit()                    { return m_Flags & (1u << 0); }
    bool IsPrimaryRay()             { return m_Flags & (1u << 1); }
    bool IsReflectionRay()          { return m_Flags & (1u << 2); }
    bool IsRefractionRay()          { return m_Flags & (1u << 3); }
    bool IsScreenRay()              { return m_Flags & (1u << 4); }

    void SetHit(bool v)             { v ? m_Flags |= (1u << 0) : m_Flags &= ~(1u << 0); }
    void SetPrimaryRay(bool v)      { v ? m_Flags |= (1u << 1) : m_Flags &= ~(1u << 1); }
    void SetReflectionRay(bool v)   { v ? m_Flags |= (1u << 2) : m_Flags &= ~(1u << 2); }
    void SetRefractionRay(bool v)   { v ? m_Flags |= (1u << 3) : m_Flags &= ~(1u << 3); }
    void SetScreenRay(bool v)       { v ? m_Flags |= (1u << 4) : m_Flags &= ~(1u << 4); }
};

struct GIPackedReservoir
{
    ethVector4 m_PackedNormals;
    ethVector3 m_WeightSum;
    ethVector3 m_TargetPdf;

    uint32_t m_PackedData0; // [fp16] VisibleDepth  | [fp16] MaterialID
    uint32_t m_PackedData1; // [fp16] M             | [fp16] Position.x
    uint32_t m_PackedData2; // [fp16] Position.y    | [fp16] Position.z
    uint32_t m_PackedData3; // [fp16] Radiance.x    | [fp16] Radiance.y
    uint32_t m_PackedData4; // [fp16] Radiance.z    | Unused
};

struct SpatialHashPayload
{
    ethVector3 m_Color;
};


#ifdef __HLSL__

#include "common/vertexcommon.h"

RaytracingAccelerationStructure RTRaytracingTlas    : register(t1);
StructuredBuffer<GeometryInfo> RTGeometryInfo       : register(t2);

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

#endif // __HLSL__

ETH_END_SHADER_NAMESPACE
