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

#ifndef __GI_RESERVOIR_SAMPLING_HLSL__
#define __GI_RESERVOIR_SAMPLING_HLSL__

#include "utils/sampling.hlsl"
#include "utils/raytracing.hlsl"
#include "utils/helpers.hlsl"
#include "common/raytracingconstants.h"
#include "common/material.h"
#include "lighting/restir/gireservoirmanagement.hlsl"

#define THREADGROUP_SIZE 8
#define DOWNSAMPLE_FACTOR 1

// TODO: Make into cvar
#define MAX_TEMPORAL_HISTORY 32
#define NUM_SPATIAL_SAMPLES 8
#define SPATIAL_KERNEL_RADIUS 64

Texture2D<float> g_SceneDepth                               : register(t3);
Texture2D<float4> g_GBufferA                                : register(t4);
Texture2D<float4> g_GBufferB                                : register(t5);
Texture2D<float4> g_GBufferC                                : register(t6);

RWStructuredBuffer<GIPackedReservoir> g_InputReservoir      : register(u0);
RWStructuredBuffer<GIPackedReservoir> g_HistoryReservoir    : register(u1);
RWStructuredBuffer<GIPackedReservoir> g_RWOutputReservoir   : register(u2);

uint2 GetSampleCoordsFromScreenCoords(uint2 screenCoords)
{
    return screenCoords / (float)DOWNSAMPLE_FACTOR;
}

uint2 GetScreenCoordsFromSampleCoords(uint2 sampleCoords)
{
    return sampleCoords * (float)DOWNSAMPLE_FACTOR;
}

uint GetSampleIndexFromSampleCoords(uint2 sampleCoords, uint2 bufferSize)
{
    return sampleCoords.y * bufferSize.x + sampleCoords.x;
}

uint GetSampleIndexFromScreenCoords(uint2 screenCoords, uint2 screenSize)
{
    return GetSampleIndexFromSampleCoords(GetSampleCoordsFromScreenCoords(screenCoords), screenSize / (float)DOWNSAMPLE_FACTOR);
}

float3 ComputeTargetFunction(ShadingSurface surface, GIReservoirSample sample)
{
    const float3 wi = normalize(sample.m_SamplePosition - surface.m_Position);
    const float3 wo = normalize(g_GlobalConstants.m_CameraPosition.xyz - surface.m_Position);
    return clamp(ComputeRadiance(surface, sample.m_Radiance, wi, wo), 0, 100000);
}

RayPayload TraceValidationRay(ShadingSurface surface, GIReservoirSample sample)
{
    RayPayload payload;
    payload.m_IsShadowRay = true;
    payload.m_Depth = 1;

    RayDesc ray;
    ray.Origin = surface.m_Position + surface.m_Normal * 0.01;
    ray.Direction = normalize(sample.m_SamplePosition - surface.m_Position);
    ray.TMax = length(sample.m_SamplePosition - ray.Origin) * 0.99f;
    ray.TMin = RAY_TMIN;
    TraceRay(g_RaytracingTlas, RAY_FLAG_FORCE_OPAQUE | RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH, 0xFF, 0, 0, 0, ray, payload);

    return payload;
}

#endif // __GI_RESERVOIR_SAMPLING_HLSL__
