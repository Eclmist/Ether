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
#include "graphics/rhi/rhishader.h"
#include "graphics/rhi/rhiraytracingpipelinestate.h"

static const wchar_t* kRayGenShader = L"RayGeneration";
static const wchar_t* kMissShader = L"Miss";
static const wchar_t* kClosestHitShader = L"ClosestHit";
static const wchar_t* kHitGroup = L"HitGroup";
static const wchar_t* EntryPoints[] = { kRayGenShader, kMissShader, kClosestHitShader };

void Ether::Graphics::RaytracedGBufferPass::Initialize(ResourceContext& resourceContext)
{
    InitializeShaders();
    InitializeRootSignatures();
    InitializePipelineStates();
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

void Ether::Graphics::RaytracedGBufferPass::InitializeShaders()
{
    const RhiDevice& gfxDevice = GraphicCore::GetDevice();

    m_LibraryShader = gfxDevice.CreateShader({ "raytracing\\raytracing.hlsl", "", RhiShaderType::Library });
    // Manually compile shader since raytracing PSO caching has not been implemented yet
    m_LibraryShader->Compile();

    RhiLibraryShaderDesc libShaderDesc = { m_LibraryShader.get(),
                                           EntryPoints,
                                           sizeof(EntryPoints) / sizeof(EntryPoints[0]) };

    if (GraphicCore::GetGraphicConfig().GetUseShaderDaemon())
        GraphicCore::GetShaderDaemon().RegisterShader(*m_LibraryShader);
}

void Ether::Graphics::RaytracedGBufferPass::InitializeRootSignatures()
{
    std::unique_ptr<RhiRootSignatureDesc> rsDesc = GraphicCore::GetDevice().CreateRootSignatureDesc(1, 0, true);

    rsDesc->SetAsDescriptorTable(0, 0, 2, RhiShaderVisibility::All);
    rsDesc->SetDescriptorTableRange(0, 0, 1, RhiDescriptorType::Uav);
    rsDesc->SetDescriptorTableRange(0, 1, 1, RhiDescriptorType::Srv);

    m_RayGenRootSignature = rsDesc->Compile();

    rsDesc = GraphicCore::GetDevice().CreateRootSignatureDesc(0, 0, true);
    m_HitMissRootSignature = rsDesc->Compile();

    rsDesc = GraphicCore::GetDevice().CreateRootSignatureDesc(0, 0, false);
    m_GlobalRootSignature = rsDesc->Compile();
}

void Ether::Graphics::RaytracedGBufferPass::InitializePipelineStates()
{
    const RhiDevice& gfxDevice = GraphicCore::GetDevice();

    RhiRaytracingPipelineStateDesc desc;
    desc.m_RayGenShaderName = kRayGenShader;
    desc.m_MissShaderName = kMissShader;
    desc.m_ClosestHitShaderName = kClosestHitShader;
    desc.m_HitGroupName = kHitGroup;
    desc.m_MaxAttributeSize = sizeof(float) * 2; // From builtin attributes
    desc.m_MaxPayloadSize = sizeof(float) * 1;   // From payload struct in shader
    desc.m_MaxRecursionDepth = 1;
    desc.m_LibraryShaderDesc = { m_LibraryShader.get(), EntryPoints, sizeof(EntryPoints) / sizeof(EntryPoints[0]) };
    desc.m_RayGenRootSignature = m_RayGenRootSignature.get();
    desc.m_HitMissRootSignature = m_HitMissRootSignature.get();
    desc.m_GlobalRootSignature = m_GlobalRootSignature.get();
        
    m_RaytracingPipelineState = gfxDevice.CreateRaytracingPipelineState(desc);
}
