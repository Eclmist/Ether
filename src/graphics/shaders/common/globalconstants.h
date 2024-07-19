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

struct GlobalConstants
{
    // ============= Common ============= //
    ethMatrix4x4 m_ViewMatrix;
    ethMatrix4x4 m_ViewMatrixInv;
    ethMatrix4x4 m_ViewMatrixPrev;
    ethMatrix4x4 m_ProjectionMatrix;
    ethMatrix4x4 m_ProjectionMatrixInv;
    ethMatrix4x4 m_ProjectionMatrixPrev;
    ethMatrix4x4 m_ViewProjectionMatrix;
    ethMatrix4x4 m_ViewProjectionMatrixInv;
    ethMatrix4x4 m_ViewProjectionMatrixPrev;

    ethVector4 m_CameraPosition;
    ethVector4 m_CameraDirection;
    ethVector4 m_Time;

    ethVector4 m_SunDirection;
    ethVector4 m_SunColor;

    ethVector2u m_ScreenResolution;
    uint32_t m_FrameNumber;

    // ============== HDRI =============== //
    uint32_t m_HdriTextureIndex;

    // ============== TAA =============== //
    float m_TaaAccumulationFactor;
    ethVector2 m_CameraJitter;
    uint32_t m_FrameSinceLastMovement;

    // ============= Debug ============== //
    uint32_t m_RaytracedLightingDebug;
    float m_RaytracedAOIntensity;

    // ============ Samplers ============ //
    uint32_t m_SamplerIndex_Point_Clamp;
    uint32_t m_SamplerIndex_Point_Wrap;
    uint32_t m_SamplerIndex_Point_Border;
    uint32_t m_SamplerIndex_Linear_Clamp;
    uint32_t m_SamplerIndex_Linear_Wrap;
    uint32_t m_SamplerIndex_Linear_Border;

    // ============ Padding ============= //
    ethVector4 m_Padding0;
    ethVector4 m_Padding1;
    ethVector4 m_Padding2;
};

ETH_SHADER_STATIC_ASSERT(sizeof(GlobalConstants) % 256 == 0 && "CBV must be 256byte aligned");

ETH_END_SHADER_NAMESPACE
