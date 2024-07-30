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

#include "globalconstantsproducer.h"

#include "graphics/graphiccore.h"
#include "graphics/shaders/common/globalconstants.h"

DEFINE_GFX_PA(GlobalConstantsProducer)
DEFINE_GFX_CB(GlobalRingBuffer)

Ether::Graphics::GlobalConstantsProducer::GlobalConstantsProducer()
    : GraphicProducer("GlobalConstantsProducer")
{
}

void Ether::Graphics::GlobalConstantsProducer::Initialize(ResourceContext& rc)
{
}

void Ether::Graphics::GlobalConstantsProducer::GetInputOutput(ScheduleContext& schedule, ResourceContext& rc)
{
    schedule.NewCB(ACCESS_GFX_CB(GlobalRingBuffer), AlignUp(sizeof(Shader::GlobalConstants), 256) * 3);
}

void Ether::Graphics::GlobalConstantsProducer::RenderFrame(GraphicContext& ctx, ResourceContext& rc)
{
    const RenderData& renderData = GraphicCore::GetGraphicRenderer().GetRenderData();

    static ethMatrix4x4 viewMatrixPrev = renderData.m_ViewMatrix;
    static ethMatrix4x4 projMatrixPrev = renderData.m_ProjectionMatrix;
    static ethVector2 cameraJitterPrev = renderData.m_CameraJitter;
    static uint32_t lastMovedFrameNumber = 0;

    if (renderData.m_ViewMatrix != viewMatrixPrev)
        lastMovedFrameNumber = GraphicCore::GetGraphicRenderer().GetFrameNumber();

    auto alloc = GetFrameAllocator().Allocate({ sizeof(Shader::GlobalConstants), 256 });
    Shader::GlobalConstants* globalConstants = (Shader::GlobalConstants*)alloc->GetCpuHandle();
    globalConstants->m_ViewMatrix = renderData.m_ViewMatrix;
    globalConstants->m_ViewMatrixInv = renderData.m_ViewMatrix.Inversed();
    globalConstants->m_ViewMatrixPrev = viewMatrixPrev;
    globalConstants->m_ProjectionMatrix = renderData.m_ProjectionMatrix;
    globalConstants->m_ProjectionMatrixInv = renderData.m_ProjectionMatrix.Inversed();
    globalConstants->m_ProjectionMatrixPrev = projMatrixPrev;
    globalConstants->m_ViewProjectionMatrix = globalConstants->m_ProjectionMatrix * globalConstants->m_ViewMatrix;
    globalConstants->m_ViewProjectionMatrixInv = globalConstants->m_ViewMatrixInv * globalConstants->m_ProjectionMatrixInv;
    globalConstants->m_ViewProjectionMatrixPrev = globalConstants->m_ProjectionMatrixPrev * globalConstants->m_ViewMatrixPrev;
    globalConstants->m_CameraDirection = renderData.m_CameraDirection.Resize<4>();
    globalConstants->m_CameraPosition = renderData.m_CameraPosition.Resize<4>();
    globalConstants->m_SunDirection = GraphicCore::GetGraphicConfig().m_SunDirection;
    globalConstants->m_SunColor = GraphicCore::GetGraphicConfig().m_SunColor;
    globalConstants->m_Time = ethVector4(Time::GetTimeSinceStartup()) / 1000.0f;
    globalConstants->m_Time.x *= 20;
    globalConstants->m_Time.y *= 1;
    globalConstants->m_Time.z *= 0.5;
    globalConstants->m_Time.w *= 0.025;
    globalConstants->m_ScreenResolution = GraphicCore::GetGraphicConfig().GetResolution();
    globalConstants->m_FrameNumber = GraphicCore::GetGraphicRenderer().GetFrameNumber();
    globalConstants->m_TaaAccumulationFactor = GraphicCore::GetGraphicConfig().m_TemporalAAAcumulationFactor;
    globalConstants->m_CameraJitter = renderData.m_CameraJitter;
    globalConstants->m_CameraJitterPrev = cameraJitterPrev;
    globalConstants->m_FrameSinceLastMovement = lastMovedFrameNumber;

    globalConstants->m_TonemapperType = GraphicCore::GetGraphicConfig().m_TonemapperType;
    globalConstants->m_TonemapperParamA = GraphicCore::GetGraphicConfig().m_TonemapperParamA;
    globalConstants->m_TonemapperParamB = GraphicCore::GetGraphicConfig().m_TonemapperParamB;
    globalConstants->m_TonemapperParamC = GraphicCore::GetGraphicConfig().m_TonemapperParamC;
    globalConstants->m_TonemapperParamD = GraphicCore::GetGraphicConfig().m_TonemapperParamD;
    globalConstants->m_TonemapperParamE = GraphicCore::GetGraphicConfig().m_TonemapperParamE;

    globalConstants->m_RaytracedLightingDebug = GraphicCore::GetGraphicConfig().m_IsRaytracingDebugEnabled ? 1 : 0;
    globalConstants->m_SamplerIndex_Point_Clamp = GraphicCore::GetGraphicCommon().m_SamplerIndex_Point_Clamp;
    globalConstants->m_SamplerIndex_Point_Wrap = GraphicCore::GetGraphicCommon().m_SamplerIndex_Point_Wrap;
    globalConstants->m_SamplerIndex_Point_Border = GraphicCore::GetGraphicCommon().m_SamplerIndex_Point_Border;
    globalConstants->m_SamplerIndex_Linear_Clamp = GraphicCore::GetGraphicCommon().m_SamplerIndex_Linear_Clamp;
    globalConstants->m_SamplerIndex_Linear_Wrap = GraphicCore::GetGraphicCommon().m_SamplerIndex_Linear_Wrap;
    globalConstants->m_SamplerIndex_Linear_Border = GraphicCore::GetGraphicCommon().m_SamplerIndex_Linear_Border;

    StringID hdriID = GraphicCore::GetGraphicRenderer().GetRenderData().m_HdriTextureID;
    globalConstants->m_HdriTextureIndex = GraphicCore::GetBindlessDescriptorManager().GetDescriptorIndex(hdriID); 

    ctx.CopyBufferRegion(
        dynamic_cast<UploadBufferAllocation&>(*alloc).GetResource(),
        *rc.GetResource(ACCESS_GFX_CB(GlobalRingBuffer)),
        alloc->GetSize(),
        0,
        alloc->GetSize() * GraphicCore::GetGraphicDisplay().GetBackBufferIndex()
    );

    // For velocity vector calculations
    viewMatrixPrev = globalConstants->m_ViewMatrix;
    projMatrixPrev = globalConstants->m_ProjectionMatrix;
    projMatrixPrev.m_13 -= globalConstants->m_CameraJitter.x;
    projMatrixPrev.m_23 -= globalConstants->m_CameraJitter.y;
    cameraJitterPrev = globalConstants->m_CameraJitter;
}

