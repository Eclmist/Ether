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

#include "common/globalconstants.h"
#include "common/irradiancefieldparams.h"
#include "utils/constants.hlsl"
#include "utils/helpers.hlsl"

ConstantBuffer<IrradianceFieldParams> IrradianceFieldParams : register(b1);

Texture2D<float3> IrradianceFieldProbeAtlas                 : register(t0);
Texture2D<float2> IrradianceFieldProbeDepth                 : register(t1);

struct VS_OUTPUT
{
    float4 Position                 : SV_Position;
    float2 TexCoord                 : TEXCOORD0;
    nointerpolation uint ProbeIndex : TEXCOORD1;
};

float3 GetProbeWorldPosition(uint probeIndex)
{
    const uint x = probeIndex % IrradianceFieldParams.m_GridResolution.x;
    const uint temp = probeIndex / IrradianceFieldParams.m_GridResolution.x;
    const uint y = temp % IrradianceFieldParams.m_GridResolution.y;
    const uint z = temp / IrradianceFieldParams.m_GridResolution.y;

    const float3 gridPos = float3(x, y, z);
    const float3 gridExtent = float3(IrradianceFieldParams.m_GridResolution - 1) * 0.5;
    return IrradianceFieldParams.m_GridOrigin + (gridPos - gridExtent) * IrradianceFieldParams.m_GridSpacing;
}

// TODO: This calculation is probably accounting for the border wrong!!
// Also, it is not interpolating
float3 SampleProbeIrradiance(uint probeIndex, float2 uv)
{
    const uint x = probeIndex % IrradianceFieldParams.m_GridResolution.x;
    const uint temp = probeIndex / IrradianceFieldParams.m_GridResolution.x;
    const uint y = temp % IrradianceFieldParams.m_GridResolution.y;
    const uint z = temp / IrradianceFieldParams.m_GridResolution.y;

    const uint probeAtlasX = x + y * IrradianceFieldParams.m_GridResolution.x;
    const uint probeAtlasY = z;

    const float2 probeUV = uv * (IrradianceFieldParams.m_NumProbeIrradianceInteriorTexels + 1.0);

    const uint2 atlasCoord = uint2(
        probeAtlasX * IrradianceFieldParams.m_NumProbeIrradianceInteriorTexels + probeUV.x,
        probeAtlasY * IrradianceFieldParams.m_NumProbeIrradianceInteriorTexels + probeUV.y
    );

    // Debug visualize temp
    return float3(uv, 1.0f);

    return IrradianceFieldProbeAtlas[atlasCoord];
}

VS_OUTPUT VS_Main(uint vertexID : SV_VertexID, uint instanceID : SV_InstanceID)
{
    VS_OUTPUT output;

    const float2 offsets[6] =
    {
        float2(-1, -1), float2(1, -1), float2(1, 1),
        float2(-1, -1), float2(1, 1), float2(-1, 1)
    };

    const float2 uvs[6] =
    {
        float2(0, 1), float2(1, 1), float2(1, 0),
        float2(0, 1), float2(1, 0), float2(0, 0)
    };

    const float3 probeWorldPos = GetProbeWorldPosition(instanceID);
    const float3 viewDir = normalize(GlobalConstants.m_CameraPosition.xyz - probeWorldPos);

    // Create orthonormal basis for billboard
    float3 up = float3(0, 1, 0);
    float3 right = normalize(cross(up, viewDir));
    up = cross(viewDir, right);

    float3 worldPos = probeWorldPos + (right * offsets[vertexID].x + up * offsets[vertexID].y) * IrradianceFieldParams.m_VisualizeProbeRadius;
    output.Position = mul(GlobalConstants.m_ViewProjectionMatrix, float4(worldPos, 1.0));
    output.TexCoord = uvs[vertexID];
    output.ProbeIndex = instanceID;

    return output;
}

float4 PS_Main(VS_OUTPUT IN) : SV_Target
{
    float dist = length(IN.TexCoord * 2.0 - 1.0);
    if (dist > 1.0)
        discard;

    float3 probeColor = SampleProbeIrradiance(IN.ProbeIndex, IN.TexCoord);

    return float4(probeColor, 1.0);
}

#endif // __IRRADIANCE_FIELD_VISUALIZE_VSPS_HLSL__