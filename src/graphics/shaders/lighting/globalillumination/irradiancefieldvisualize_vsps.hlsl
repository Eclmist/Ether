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

#ifndef __IRRADIANCE_FIELD_VISUALIZE_VSPS_HLSL__
#define __IRRADIANCE_FIELD_VISUALIZE_VSPS_HLSL__

#include "lighting/globalillumination/irradiancefield.hlsl"

struct VS_OUTPUT
{
    float4 Position                 : SV_Position;
    float3 WorldPos                 : TEXCOORD0;
    nointerpolation uint ProbeIndex : TEXCOORD1;
};

float2 RayIntersectSphere(float3 rayOrigin, float3 rayDir, float4 sphere)
{
    float3 oc = rayOrigin - sphere.xyz;
    float b = dot(oc, rayDir);
    float c = dot(oc, oc) - sphere.w * sphere.w;
    float h = b * b - c;
    
    if (h < 0.0)
        return float2(-1.0, -1.0);
    
    h = sqrt(h);
    return float2(-b - h, -b + h);
}

VS_OUTPUT VS_Main(uint vertexID : SV_VertexID, uint instanceID : SV_InstanceID)
{
    VS_OUTPUT output;

    const float2 offsets[6] = 
    {
        float2(-1, -1), float2(1, -1), float2(1, 1),
        float2(-1, -1), float2(1, 1), float2(-1, 1)
    };

    const float3 probeWorldPos = GetProbeWorldPosition(instanceID);

    const float4 probeClipPos = mul(GlobalConstants.m_ViewProjectionMatrix, float4(probeWorldPos, 1.0));
    const float3 probeNDC = probeClipPos.xyz / probeClipPos.w;
    const float screenRadius = IrradianceFieldParams.m_VisualizeProbeRadius * 2.0f / probeClipPos.w;

    const float aspectRatio = GlobalConstants.m_ProjectionMatrix[1][1] / GlobalConstants.m_ProjectionMatrix[0][0];
    float2 ndcOffset = offsets[vertexID] * screenRadius;
    ndcOffset.x /= aspectRatio;

    const float4 vertexClipPos = float4(probeNDC.xy + ndcOffset, probeNDC.z, 1.0);
    const float4 vertexWorldPos = mul(GlobalConstants.m_ViewProjectionMatrixInv, vertexClipPos);

    output.Position = vertexClipPos;
    output.WorldPos = vertexWorldPos.xyz / vertexWorldPos.w;
    output.ProbeIndex = instanceID;

    return output;
}

float4 PS_Main(VS_OUTPUT IN) : SV_Target
{
    const float3 probeWorldCenter = GetProbeWorldPosition(IN.ProbeIndex);
    const float probeRadius = IrradianceFieldParams.m_VisualizeProbeRadius;
    const float3 rayOrigin = GlobalConstants.m_CameraPosition.xyz;
    const float3 rayDir = normalize(IN.WorldPos - rayOrigin);

    const float4 sphere = float4(probeWorldCenter, probeRadius);
    const float2 intersections = RayIntersectSphere(rayOrigin, rayDir, sphere);
    clip(intersections.x);

    const float3 intersectionPos = rayOrigin + rayDir * intersections.x;
    const float3 worldDir = normalize(intersectionPos - probeWorldCenter);
    const float3 probeColor = SampleProbeIrradiance(IN.ProbeIndex, worldDir) / Pi;

    return float4(probeColor, 1.0);
}

#endif // __IRRADIANCE_FIELD_VISUALIZE_VSPS_HLSL__