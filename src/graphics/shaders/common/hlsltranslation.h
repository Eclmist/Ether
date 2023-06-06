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

#pragma once

#ifdef __cplusplus
#define ETH_BEGIN_SHADER_NAMESPACE namespace Ether::Graphics::Shader {
#define ETH_END_SHADER_NAMESPACE }
#else
#define ETH_BEGIN_SHADER_NAMESPACE
#define ETH_END_SHADER_NAMESPACE
#endif

#ifdef __HLSL__
typedef float4x4 ethMatrix4x4;
typedef float3x3 ethMatrix3x3;

typedef float4 ethVector4;
typedef float3 ethVector3;
typedef float2 ethVector2;

typedef uint4 ethVector4u;
typedef uint3 ethVector3u;
typedef uint2 ethVector2u;
#endif

