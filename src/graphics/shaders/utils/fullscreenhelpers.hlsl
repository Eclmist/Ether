/*
    This file is part of Ether, an open-source DirectX 12 renderer.

    Copyright (c) 2020-2023 Samuel Huang - All rights reserved.

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

void GetVertexFromID(const uint vertexID, out float2 pos, out float2 uv)
{
    uint2 v = uint2(vertexID / 2, vertexID % 2);

    pos.x = v.x * 4.0f - 1.0f;
    pos.y = v.y * 4.0f - 1.0f;

    uv = float2(vertexID / 2 * 2, vertexID % 2 * 2);
    uv.y = 1 - uv.y;
}

float2 ClipToTextureSpace(float4 clip)
{
    float4 ndc = clip / clip.w;
    ndc.y = -ndc.y;
    float2 uv = (ndc.xy + 1.0f) * 0.5f;
    return uv;
}

float4 ScreenToClipSpace(float2 uv, float sceneDepth)
{
    float4 clip;
    clip.xy = uv * 2.0f - 1.0f;
    clip.z = sceneDepth;
    clip.w = 1.0f;
    return clip;
}

float3 ClipToWorldSpace(float4 clip, float4x4 invViewProj)
{
    float4 worldPos = mul(invViewProj, clip);
    worldPos.xyz /= worldPos.w;
    return worldPos.xyz;
}

float3 ScreenToWorldSpace(float2 uv, float sceneDepth, float4x4 invViewProj)
{
    return ClipToWorldSpace(ScreenToClipSpace(uv, sceneDepth), invViewProj);
}
