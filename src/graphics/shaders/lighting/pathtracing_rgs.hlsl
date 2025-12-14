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

#ifndef __PATHTRACING_RGS_HLSL__
#define __PATHTRACING_RGS_HLSL__

#include "utils/helpers.hlsl"
#include "utils/sampling.hlsl"
#include "utils/raytracing.hlsl"

Texture2D<float4> GBufferTextureA                       : register(t3);
Texture2D<float4> GBufferTextureB                       : register(t4);
Texture2D<float4> GBufferTextureC                       : register(t5);
Texture2D<float2> SceneDepth                            : register(t6);
Texture2D<float4> DiffuseIndirectAccumulationTexture    : register(t7);
RWTexture2D<float4> RWDirectLightingTexture             : register(u0);
RWTexture2D<float4> RWDiffuseIndirectLightingTexture    : register(u1);

[shader("raygeneration")]
void RayGeneration()
{
    const float2 screenCoords = DispatchRaysIndex().xy;
    const uint2 bufferSize = DispatchRaysDimensions().xy;
    const uint sampleIdx = screenCoords.y * bufferSize.x + screenCoords.x;
    const ShadingSurface surface = GetShadingSurfaceFromGBuffers(screenCoords, GBufferTextureA, GBufferTextureB, GBufferTextureC, SceneDepth);

    const float3 wo = normalize(GlobalConstants.m_CameraPosition.xyz - surface.m_Position);
    const float2 uv = ScreenToTextureSpace(screenCoords);
    const float2 uvPrev = ScreenToTextureSpace(screenCoords - surface.m_Velocity);
    float4 accumulation = 0.0f;

    sampler linearSampler = SamplerDescriptorHeap[GlobalConstants.m_SamplerIndex_Linear_Clamp];
    if (all(uvPrev >= 0.0f) && all(uvPrev <= 1.0f))
    {
        accumulation = DiffuseIndirectAccumulationTexture.SampleLevel(linearSampler, uvPrev, 0);

        if (any(isnan(accumulation)) || any(isinf(accumulation)))
            accumulation = 0.0f;
        
    }

    const RayPayload shadowRay = TraceShadowRay(surface, GlobalConstants.m_SunDirection.xyz);
    const float3 direct = ComputeRadiance(surface, shadowRay.m_Radiance, GlobalConstants.m_SunDirection.xyz, wo);

    float3 wi;
    float pdf;
    float3 indirect = 0.0f;

#if USE_IMPORTANCE_SAMPLING
    SampleDirectionBrdf(surface, GlobalConstants.m_FrameNumber, wo, wi, pdf);
#else
    SampleDirectionUniform(surface, GlobalConstants.m_FrameNumber, wi, pdf);
#endif

    if (pdf > 0.01f)
    {
        const RayPayload indirectRay = TraceShadingRay(surface, wi, MAX_DEPTH);
        indirect = ComputeRadiance(surface, indirectRay.m_Radiance, wi, wo) / pdf;
    }

    float a = max(0.005, 1 - smoothstep(0, 10, GlobalConstants.m_FrameNumber - GlobalConstants.m_FrameSinceLastMovement));
    const float3 accumulatedIndirect = (a * indirect) + (1 - a) * accumulation.xyz;
    RWDirectLightingTexture[screenCoords].xyz = surface.m_Emission + direct;
    RWDiffuseIndirectLightingTexture[screenCoords].xyz = accumulatedIndirect;
}

#endif // __PATHTRACING_RGS_HLSL__
