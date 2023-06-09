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

#include "graphics/schedule/TempRaytracingFrameDump.h"

#include "graphics/graphiccore.h"
#include "graphics/rhi/rhishader.h"
#include "graphics/rhi/rhiraytracingpipelinestate.h"
#include "graphics/rhi/rhiresourceviews.h"
#include "graphics/shaders/common/globalconstants.h"
#include "graphics/shaders/common/raypayload.h"

static const wchar_t* kRayGenShader = L"RayGeneration";
static const wchar_t* kMissShader = L"Miss";
static const wchar_t* kClosestHitShader = L"ClosestHit";
static const wchar_t* kHitGroup = L"HitGroup";
static const wchar_t* EntryPoints[] = { kRayGenShader, kMissShader, kClosestHitShader };

namespace Ether::Graphics::RhiLinkSpace
{
extern RhiGpuAddress g_GlobalConstantsCbv;
}

void Ether::Graphics::TempRaytracingFrameDump::Initialize(ResourceContext& rc)
{
    InitializeShaders();
    InitializeRootSignatures();
    InitializePipelineStates();
}

void Ether::Graphics::TempRaytracingFrameDump::FrameSetup(ResourceContext& rc)
{
    ethVector2u resolution = GraphicCore::GetGraphicConfig().GetResolution();

    if (!m_Shader->IsCompiled())
    {
        m_Shader->Compile();
        GraphicCore::FlushGpu();
        InitializeRootSignatures();
        InitializePipelineStates();
    }

    m_OutputTexture = &rc.CreateTexture2DUavResource("Raytrace - Output Texture", resolution, RhiFormat::R32G32B32A32Float);
    m_OutputTextureUav = rc.CreateUnorderedAccessView("Raytrace - Output UAV", m_OutputTexture, RhiFormat::R32G32B32A32Float, RhiUnorderedAccessDimension::Texture2D);
    m_AccumulationSrv = rc.CreateShaderResourceView("Raytrace - Accumulation SRV", m_OutputTexture, RhiFormat::R32G32B32A32Float, RhiShaderResourceDimension::Texture2D);

    m_GBufferSrv0 = rc.GetView<RhiShaderResourceView>("GBuffer - SRV 0");
    m_GBufferSrv1 = rc.GetView<RhiShaderResourceView>("GBuffer - SRV 1");
    m_GBufferSrv2 = rc.GetView<RhiShaderResourceView>("GBuffer - SRV 2");
}

void Ether::Graphics::TempRaytracingFrameDump::Render(GraphicContext& ctx, ResourceContext& rc)
{
    ETH_MARKER_EVENT("Raytraced GBuffer Pass - Render");
    const RhiDevice& gfxDevice = GraphicCore::GetDevice();
    const GraphicDisplay& gfxDisplay = GraphicCore::GetGraphicDisplay();
    const GraphicConfig& config = GraphicCore::GetGraphicConfig();

    if (ctx.GetVisualBatch().m_Visuals.empty())
        return;

    m_TopLevelAccelerationStructure = &rc.CreateAccelerationStructure(
        "Raytracing TLAS",
        { (void*)&ctx.GetVisualBatch() });
    m_TlasSrv = rc.CreateAccelerationStructureView("Raytracing TLAS SRV", m_TopLevelAccelerationStructure);

    InitializeShaderBindingTable(rc);

    const RhiResourceView* gbufferSrvs[] = { &m_GBufferSrv0, &m_GBufferSrv1, &m_GBufferSrv2 };
    m_GlobalRootTableAlloc = std::move(GraphicCore::GetSrvCbvUavAllocator().Commit(gbufferSrvs, 3));
    GraphicCore::GetBindlessDescriptorManager().RegisterAsShaderResourceView("GBuffer - SRV 0", m_GlobalRootTableAlloc->GetOffset());
    GraphicCore::GetBindlessDescriptorManager().RegisterAsShaderResourceView("GBuffer - SRV 1", m_GlobalRootTableAlloc->GetOffset() + 1);
    GraphicCore::GetBindlessDescriptorManager().RegisterAsShaderResourceView("GBuffer - SRV 2", m_GlobalRootTableAlloc->GetOffset() + 2);

    ctx.PushMarker("Clear");
    ctx.TransitionResource(gfxDisplay.GetBackBuffer(), RhiResourceState::RenderTarget);
    ctx.ClearColor(gfxDisplay.GetBackBufferRtv(), config.GetClearColor());
    ctx.PopMarker();

    const auto resolution = GraphicCore::GetGraphicConfig().GetResolution();
    ctx.PushMarker("Raytrace");
    ctx.TransitionResource(*m_OutputTexture, RhiResourceState::UnorderedAccess);
    ctx.SetDescriptorHeap(GraphicCore::GetSrvCbvUavAllocator().GetDescriptorHeap());
    ctx.SetComputeRootSignature(*m_GlobalRootSignature);
    ctx.SetComputeRootConstantBufferView(0, RhiLinkSpace::g_GlobalConstantsCbv);
    ctx.SetComputeRootDescriptorTable(1, ((DescriptorAllocation&)*m_GlobalRootTableAlloc).GetGpuAddress());
    ctx.SetRaytracingShaderBindingTable(m_RaytracingShaderBindingTable[gfxDisplay.GetBackBufferIndex()]);
    ctx.SetRaytracingPipelineState(*m_RaytracingPipelineState);
    ctx.DispatchRays(resolution.x, resolution.y, 1);
    ctx.PopMarker();
}

void Ether::Graphics::TempRaytracingFrameDump::Reset()
{
}

void Ether::Graphics::TempRaytracingFrameDump::InitializeShaders()
{
    const RhiDevice& gfxDevice = GraphicCore::GetDevice();

    m_Shader = gfxDevice.CreateShader({ "raytracing\\raytracing.hlsl", "", RhiShaderType::Library });
    // Manually compile shader since raytracing PSO caching has not been implemented yet
    m_Shader->Compile();

    if (GraphicCore::GetGraphicConfig().GetUseShaderDaemon())
        GraphicCore::GetShaderDaemon().RegisterShader(*m_Shader);
}

void Ether::Graphics::TempRaytracingFrameDump::InitializeRootSignatures()
{
    std::unique_ptr<RhiRootSignatureDesc> rsDesc = GraphicCore::GetDevice().CreateRootSignatureDesc(1, 0, true);

    rsDesc->SetAsDescriptorTable(0, 2, RhiShaderVisibility::All);
    rsDesc->SetDescriptorTableRange(0, RhiDescriptorType::Srv, 2, 0);
    rsDesc->SetDescriptorTableRange(0, RhiDescriptorType::Uav, 1, 1);
    m_RayGenRootSignature = rsDesc->Compile("Ray Generation Root Signature (Local)");

    rsDesc = GraphicCore::GetDevice().CreateRootSignatureDesc(0, 0, true);
    m_HitMissRootSignature = rsDesc->Compile("Empty Root Signature (Local)");

    rsDesc = GraphicCore::GetDevice().CreateRootSignatureDesc(2, 2, false);
    rsDesc->SetAsConstantBufferView(0, 0, RhiShaderVisibility::All);
    rsDesc->SetAsDescriptorTable(1, 1, RhiShaderVisibility::All);
    rsDesc->SetDescriptorTableRange(1, RhiDescriptorType::Srv, 3, 0, 2);
    rsDesc->SetAsSampler(0, GraphicCore::GetGraphicCommon().m_PointSampler, RhiShaderVisibility::All);
    rsDesc->SetAsSampler(1, GraphicCore::GetGraphicCommon().m_BilinearSampler, RhiShaderVisibility::All);

    m_GlobalRootSignature = rsDesc->Compile("Raytracing Root Signature (Global)");
}

void Ether::Graphics::TempRaytracingFrameDump::InitializePipelineStates()
{
    const RhiDevice& gfxDevice = GraphicCore::GetDevice();

    RhiRaytracingPipelineStateDesc desc = {};
    desc.m_RayGenShaderName = kRayGenShader;
    desc.m_MissShaderName = kMissShader;
    desc.m_ClosestHitShaderName = kClosestHitShader;
    desc.m_HitGroupName = kHitGroup;
    desc.m_MaxAttributeSize = sizeof(float) * 2; // From builtin attributes
    desc.m_MaxPayloadSize = sizeof(Shader::RayPayload);
    desc.m_MaxRecursionDepth = 1;
    desc.m_LibraryShaderDesc = { m_Shader.get(), EntryPoints, sizeof(EntryPoints) / sizeof(EntryPoints[0]) };
    desc.m_RayGenRootSignature = m_RayGenRootSignature.get();
    desc.m_HitMissRootSignature = m_HitMissRootSignature.get();
    desc.m_GlobalRootSignature = m_GlobalRootSignature.get();

    m_RaytracingPipelineState = gfxDevice.CreateRaytracingPipelineState(desc);
}

void Ether::Graphics::TempRaytracingFrameDump::InitializeShaderBindingTable(ResourceContext& rc)
{
    const GraphicDisplay& gfxDisplay = GraphicCore::GetGraphicDisplay();

    const RhiResourceView* raygenDescriptors[] = { &m_TlasSrv, &m_AccumulationSrv, &m_OutputTextureUav };
    m_RaygenRootTableAlloc = std::move(GraphicCore::GetSrvCbvUavAllocator().Commit(raygenDescriptors, sizeof(raygenDescriptors) / sizeof(raygenDescriptors[0])));
    GraphicCore::GetBindlessDescriptorManager().RegisterAsShaderResourceView("Raytrace - Output UAV", m_RaygenRootTableAlloc->GetOffset() + 2);

    RhiRaytracingShaderBindingTableDesc desc = {};
    desc.m_MaxRootSignatureSize = 8; // Raygen's descriptor table1 (8)
    desc.m_RaytracingPipelineState = m_RaytracingPipelineState.get();
    desc.m_HitGroupName = kHitGroup;
    desc.m_MissShaderName = kMissShader;
    desc.m_RayGenShaderName = kRayGenShader;
    desc.m_RayGenRootTableAddress = ((DescriptorAllocation&)*m_RaygenRootTableAlloc).GetGpuAddress();

    m_RaytracingShaderBindingTable[gfxDisplay.GetBackBufferIndex()] = &rc.CreateRaytracingShaderBindingTable(
        ("Raytracing Shader Bindings Table" + std::to_string(gfxDisplay.GetBackBufferIndex())).c_str(),
        desc);
}
