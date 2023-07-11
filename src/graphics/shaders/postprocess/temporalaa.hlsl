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

#include "common/globalconstants.h"
#include "utils/fullscreenhelpers.hlsl"

ConstantBuffer<GlobalConstants> g_GlobalConstants   : register(b0);
Texture2D<float4> g_GBufferTexture0                 : register(t0);
RWTexture2D<float4> g_TargetTexture                 : register(u0);

[numthreads(32, 32, 1)]
void CS_Main(uint3 threadID : SV_DispatchThreadID)
{
    float4 col = g_TargetTexture[threadID.xy];
    //col = 1 - col;
    g_TargetTexture[threadID.xy] = col;
}