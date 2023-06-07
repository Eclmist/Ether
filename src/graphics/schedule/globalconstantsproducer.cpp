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

#include "graphics/graphiccore.h"
#include "graphics/schedule/globalconstantsproducer.h"
#include "graphics/shaders/common/globalconstants.h"

namespace Ether::Graphics::RhiLinkSpace
{
RhiGpuAddress g_GlobalConstantsCbv;
}

void Ether::Graphics::GlobalConstantsProducer::Initialize(ResourceContext& resourceContext)
{
    for (int i = 0; i < MaxSwapChainBuffers; ++i)
        m_FrameLocalUploadBuffer[i] = std::make_unique<UploadBufferAllocator>(_2MiB);

    RhiDevice& gfxDevice = GraphicCore::GetDevice();
    GraphicDisplay& gfxDisplay = GraphicCore::GetGraphicDisplay();
}

void Ether::Graphics::GlobalConstantsProducer::FrameSetup(ResourceContext& resourceContext)
{
}

void Ether::Graphics::GlobalConstantsProducer::Render(GraphicContext& graphicContext, ResourceContext& resourceContext)
{
    ETH_MARKER_EVENT("GlobalConstantsProducer - Populate Upload Buffer");
    const RhiDevice& gfxDevice = GraphicCore::GetDevice();
    const GraphicDisplay& gfxDisplay = GraphicCore::GetGraphicDisplay();
    const GraphicConfig& config = GraphicCore::GetGraphicConfig();

    Shader::GlobalConstants globalConstants;
    globalConstants.m_ViewMatrix = graphicContext.GetViewMatrix();
    globalConstants.m_ProjectionMatrix = graphicContext.GetProjectionMatrix();
    globalConstants.m_EyeDirection = graphicContext.GetEyeDirection().Resize<4>();
    globalConstants.m_EyePosition = graphicContext.GetEyePosition().Resize<4>();
    globalConstants.m_Time = ethVector4(
        Time::GetTimeSinceStartup() / 20,
        Time::GetTimeSinceStartup(),
        Time::GetTimeSinceStartup() * 2,
        Time::GetTimeSinceStartup() * 3);
    globalConstants.m_ScreenResolution = GraphicCore::GetGraphicConfig().GetResolution();
    globalConstants.m_TemporalAccumulationFactor = GraphicCore::GetGraphicConfig().m_TemporalAccumulation;

    auto alloc = m_FrameLocalUploadBuffer[GraphicCore::GetGraphicDisplay().GetBackBufferIndex()]->Allocate(
        { sizeof(Shader::GlobalConstants), 256 });
    memcpy(alloc->GetCpuHandle(), &globalConstants, sizeof(Shader::GlobalConstants));

    RhiLinkSpace::g_GlobalConstantsCbv = dynamic_cast<UploadBufferAllocation&>(*alloc).GetGpuAddress();
}

void Ether::Graphics::GlobalConstantsProducer::Reset()
{
    GraphicDisplay& gfxDisplay = GraphicCore::GetGraphicDisplay();
    m_FrameLocalUploadBuffer[gfxDisplay.GetBackBufferIndex()]->Reset();
}

