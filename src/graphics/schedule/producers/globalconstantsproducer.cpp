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
    schedule.NewCB(ACCESS_GFX_CB(GlobalRingBuffer), sizeof(Shader::GlobalConstants) * 3);
}

void Ether::Graphics::GlobalConstantsProducer::RenderFrame(GraphicContext& ctx, ResourceContext& rc)
{
    const RenderData& renderData = GraphicCore::GetGraphicRenderer().GetRenderData();

    static ethMatrix4x4 prevViewMatrix = renderData.m_ViewMatrix;
    static ethMatrix4x4 prevProjMatrix = renderData.m_ProjectionMatrix;

    auto alloc = GetFrameAllocator().Allocate({ sizeof(Shader::GlobalConstants), 256 });
    Shader::GlobalConstants* globalConstants = (Shader::GlobalConstants*)alloc->GetCpuHandle();
    globalConstants->m_ViewMatrix = renderData.m_ViewMatrix;
    globalConstants->m_ViewMatrixInv = renderData.m_ViewMatrix.Inversed();
    globalConstants->m_ViewMatrixPrev = prevViewMatrix;
    globalConstants->m_ProjectionMatrix = renderData.m_ProjectionMatrix;
    globalConstants->m_ProjectionMatrixInv = renderData.m_ProjectionMatrix.Inversed();
    globalConstants->m_ProjectionMatrixPrev = prevProjMatrix;
    globalConstants->m_ViewProjectionMatrix = globalConstants->m_ProjectionMatrix * globalConstants->m_ViewMatrix;
    globalConstants->m_ViewProjectionMatrixInv = globalConstants->m_ViewMatrixInv * globalConstants->m_ProjectionMatrixInv;
    globalConstants->m_ViewProjectionMatrixPrev = globalConstants->m_ProjectionMatrixPrev * globalConstants->m_ViewMatrixPrev;
    globalConstants->m_EyeDirection = renderData.m_EyeDirection.Resize<4>();
    globalConstants->m_EyePosition = renderData.m_EyePosition.Resize<4>();
    globalConstants->m_SunDirection = GraphicCore::GetGraphicConfig().m_SunDirection;
    globalConstants->m_SunColor = GraphicCore::GetGraphicConfig().m_SunColor;
    globalConstants->m_Time = ethVector4(Time::GetTimeSinceStartup());
    globalConstants->m_Time.x *= 20;
    globalConstants->m_Time.y *= 1;
    globalConstants->m_Time.z *= 0.5;
    globalConstants->m_Time.w *= 0.25;
    globalConstants->m_ScreenResolution = GraphicCore::GetGraphicConfig().GetResolution();
    globalConstants->m_FrameNumber = GraphicCore::GetGraphicRenderer().GetFrameNumber();
    globalConstants->m_TaaAccumulationFactor = GraphicCore::GetGraphicConfig().m_TemporalAAAcumulationFactor;
    globalConstants->m_RaytracedLightingDebug = GraphicCore::GetGraphicConfig().m_IsRaytracingDebugEnabled ? 1 : 0;
    globalConstants->m_RaytracedAOIntensity = GraphicCore::GetGraphicConfig().m_RaytracedAOIntensity;
    globalConstants->m_SamplerIndex_Point_Clamp = GraphicCore::GetGraphicCommon().m_SamplerIndex_Point_Clamp;
    globalConstants->m_SamplerIndex_Point_Wrap = GraphicCore::GetGraphicCommon().m_SamplerIndex_Point_Wrap;
    globalConstants->m_SamplerIndex_Point_Border = GraphicCore::GetGraphicCommon().m_SamplerIndex_Point_Border;
    globalConstants->m_SamplerIndex_Linear_Clamp = GraphicCore::GetGraphicCommon().m_SamplerIndex_Linear_Clamp;
    globalConstants->m_SamplerIndex_Linear_Wrap = GraphicCore::GetGraphicCommon().m_SamplerIndex_Linear_Wrap;
    globalConstants->m_SamplerIndex_Linear_Border = GraphicCore::GetGraphicCommon().m_SamplerIndex_Linear_Border;

    ctx.CopyBufferRegion(
        dynamic_cast<UploadBufferAllocation&>(*alloc).GetResource(),
        *rc.GetResource(ACCESS_GFX_CB(GlobalRingBuffer)),
        sizeof(Shader::GlobalConstants),
        0,
        sizeof(Shader::GlobalConstants) * GraphicCore::GetGraphicDisplay().GetBackBufferIndex()
    );

    // For velocity vector calculations
    prevViewMatrix = globalConstants->m_ViewMatrix;
    prevProjMatrix = globalConstants->m_ProjectionMatrix;
}

