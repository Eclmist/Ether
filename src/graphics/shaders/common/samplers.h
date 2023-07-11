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

#include "hlsltranslation.h"

ETH_BEGIN_SHADER_NAMESPACE

struct Samplers
{

    ethVector4 m_Padding1;
    ethVector4 m_Padding2;
    ethVector4 m_Padding3;
    ethVector4 m_Padding4;
    ethVector4 m_Padding5;
    ethVector4 m_Padding6;
    ethVector4 m_Padding7;
    ethVector4 m_Padding8;
    ethVector4 m_Padding9;
    ethVector4 m_Padding10;
    ethVector4 m_Padding11;
    ethVector4 m_Padding12;
    ethVector4 m_Padding13;
    ethVector4 m_Padding14;
    ethVector2 m_Padding15;
};

ETH_SHADER_STATIC_ASSERT(sizeof(Samplers) % 256 == 0 && "CBV must be 256byte aligned");

#ifdef __HLSL__
ConstantBuffer<Samplers> g_Samplers : register(b0);
sampler g_PointClampSampler = ResourceDescriptorHeap[g_Samplers.m_SamplerIndex_Point_Clamp];
sampler g_PointWrapSampler = ResourceDescriptorHeap[g_Samplers.m_SamplerIndex_Point_Wrap];
sampler g_PointBorderSampler = ResourceDescriptorHeap[g_Samplers.m_SamplerIndex_Point_Border];
sampler g_LinearClampSampler = ResourceDescriptorHeap[g_Samplers.m_SamplerIndex_Linear_Clamp];
sampler g_LinearWrapSampler = ResourceDescriptorHeap[g_Samplers.m_SamplerIndex_Linear_Wrap];
sampler g_LinearBorderSampler = ResourceDescriptorHeap[g_Samplers.m_SamplerIndex_Linear_Border];
#endif // __HLSL__

ETH_END_SHADER_NAMESPACE
