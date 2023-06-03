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

#include "graphics/schedule/raytracedgbufferpass.h"
#include "graphics/graphiccore.h"

void Ether::Graphics::RaytracedGBufferPass::Initialize(ResourceContext& resourceContext)
{
    const RhiDevice& gfxDevice = GraphicCore::GetDevice();

    const wchar_t* entryPoints[4] = { L"RayGeneration", L"Miss", L"ClosestHit", L"HitGroup" };
    m_LibraryShaderBlob = gfxDevice.CreateShader({ "raytracing\\raytracing.hlsl", "", RhiShaderType::Library });

    RhiLibraryShaderDesc libShaderDesc = { m_LibraryShaderBlob.get(),
                                           entryPoints,
                                           sizeof(entryPoints) / sizeof(entryPoints[0]) };
    m_LibraryShader = gfxDevice.CreateLibraryShader(libShaderDesc);

    if (GraphicCore::GetGraphicConfig().GetUseShaderDaemon())
        GraphicCore::GetShaderDaemon().RegisterShader(*m_LibraryShaderBlob);
}

void Ether::Graphics::RaytracedGBufferPass::FrameSetup(ResourceContext& resourceContext)
{
}

void Ether::Graphics::RaytracedGBufferPass::Render(GraphicContext& graphicContext, ResourceContext& resourceContext)
{
    ETH_MARKER_EVENT("Raytraced GBuffer Pass - Render");
    const RhiDevice& gfxDevice = GraphicCore::GetDevice();
    const GraphicDisplay& gfxDisplay = GraphicCore::GetGraphicDisplay();
    const GraphicConfig& config = GraphicCore::GetGraphicConfig();

    RhiTopLevelAccelerationStructureDesc desc = {};
    desc.m_VisualBatch = (void*)(&graphicContext.GetVisualBatch());
    m_TopLevelAccelerationStructure = GraphicCore::GetDevice().CreateAccelerationStructure(desc);

    CommandContext tlasBuildContext(RhiCommandType::Graphic, "TLAS Build Context - Build GBuffer TLAS");
    tlasBuildContext.PushMarker("Build TLAS");
    tlasBuildContext.TransitionResource(
        *m_TopLevelAccelerationStructure->m_ScratchBuffer,
        RhiResourceState::UnorderedAccess);
    tlasBuildContext.BuildBottomLevelAccelerationStructure(*m_TopLevelAccelerationStructure);
    tlasBuildContext.FinalizeAndExecute(true);
    tlasBuildContext.Reset();

    graphicContext.PushMarker("Clear");
    graphicContext.SetViewport(gfxDisplay.GetViewport());
    graphicContext.SetScissorRect(gfxDisplay.GetScissorRect());
    graphicContext.TransitionResource(gfxDisplay.GetBackBuffer(), RhiResourceState::RenderTarget);
    graphicContext.ClearColor(gfxDisplay.GetBackBufferRtv(), config.GetClearColor());
    // graphicContext.ClearDepthStencil(*m_Dsv, 1.0);
    graphicContext.PopMarker();
    graphicContext.FinalizeAndExecute();
    graphicContext.Reset();
}

void Ether::Graphics::RaytracedGBufferPass::Reset()
{
}
