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

struct ReSTIRConstants
{
#ifndef __HLSL__
    ReSTIRConstants()
    {
        m_UseTemporalResampling = 1;
        m_UseSpatialResampling = 1;
        m_UseSpatialAccumulation = 0;
        m_UseInitialImportanceSampling = 1;
        m_UseBounceImportanceSampling = 0;
        m_UseBrdfInTargetFunction = 1;
        m_UseJacobianDeterminant = 0;
        m_UseReservoirLengthClamping = 1;

        m_TemporalHistoryLength = 30;
        m_SpatialHistoryLength = 500;
        m_SpatialResamplingRadius = 20;

        m_MaxNumBounces = 0;
        m_ClearHistoryOnMovement = 0;
    }
#endif

    uint32_t m_UseTemporalResampling;
    uint32_t m_UseSpatialResampling;
    uint32_t m_UseSpatialAccumulation;
    uint32_t m_UseInitialImportanceSampling;
    uint32_t m_UseBounceImportanceSampling;
    uint32_t m_UseBrdfInTargetFunction;
    uint32_t m_UseJacobianDeterminant;
    uint32_t m_UseReservoirLengthClamping;

    uint32_t m_TemporalHistoryLength;
    uint32_t m_SpatialHistoryLength;
    uint32_t m_SpatialResamplingRadius;

    uint32_t m_MaxNumBounces;

    // Debug
    uint32_t m_ClearHistoryOnMovement;
};

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

    // =========== TONEMAPPER =========== //
    uint32_t m_TonemapperType;
    float m_TonemapperParamA;
    float m_TonemapperParamB;
    float m_TonemapperParamC;
    float m_TonemapperParamD;
    float m_TonemapperParamE;
    float m_TonemapperParamF;

    // =========== RAYTRACING ============ //
    float m_RaytracedLightingDebug;
    float m_EmissiveScale;
    float m_SunlightScale;
    float m_SkylightScale;
    float m_LocallightScale;
    uint32_t m_MaxNumBounces;

    // ============ SAMPLERS ============ //
    uint32_t m_SamplerIndex_Point_Clamp;
    uint32_t m_SamplerIndex_Point_Wrap;
    uint32_t m_SamplerIndex_Point_Border;
    uint32_t m_SamplerIndex_Linear_Clamp;
    uint32_t m_SamplerIndex_Linear_Wrap;
    uint32_t m_SamplerIndex_Linear_Border;

    // ReSTIR
    ReSTIRConstants m_ReSTIRConstants;

    // ============ Padding ============= //
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
};

ETH_SHADER_STATIC_ASSERT(sizeof(GlobalConstants) % 256 == 0 && "CBV must be 256byte aligned");

ETH_END_SHADER_NAMESPACE
