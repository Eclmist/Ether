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
#include "graphics/rhi/rhiresourceviews.h"

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
    static RhiCommitedResourceDesc prevDesc = {};

    RhiCommitedResourceDesc outputTextureDesc = {};
    outputTextureDesc.m_Name = "RaytracedGBuffer::OutputTexture";
    outputTextureDesc.m_HeapType = RhiHeapType::Default;
    outputTextureDesc.m_State = RhiResourceState::CopySrc;
    outputTextureDesc.m_ResourceDesc = RhiCreateTexture2DResourceDesc(
        RhiFormat::R8G8B8A8Unorm,
        GraphicCore::GetGraphicConfig().GetResolution());
    outputTextureDesc.m_ResourceDesc.m_Flag = RhiResourceFlag::AllowUnorderedAccess;

    if (std::memcmp(&prevDesc.m_ResourceDesc, &outputTextureDesc.m_ResourceDesc, sizeof(RhiResourceDesc)) == 0)
        return;
    prevDesc = outputTextureDesc;

    m_OutputTexture = GraphicCore::GetDevice().CreateCommittedResource(outputTextureDesc);

    auto uavAlloc = GraphicCore::GetSrvCbvUavAllocator().Allocate();
    RhiUnorderedAccessViewDesc uavDesc = {};
    uavDesc.m_Format = RhiFormat::R8G8B8A8Unorm;
    uavDesc.m_Resource = m_OutputTexture.get();
    uavDesc.m_TargetCpuAddress = ((DescriptorAllocation&)(*uavAlloc)).GetCpuAddress();
    uavDesc.m_Dimensions = RhiUnorderedAccessDimension::Texture2D;

    m_OutputTextureUav = GraphicCore::GetDevice().CreateUnorderedAccessView(uavDesc);
    GraphicCore::GetSrvCbvUavAllocator().Free(std::move(uavAlloc));
}

void Ether::Graphics::RaytracedGBufferPass::Render(GraphicContext& graphicContext, ResourceContext& resourceContext)
{
    ETH_MARKER_EVENT("Raytraced GBuffer Pass - Render");
    const RhiDevice& gfxDevice = GraphicCore::GetDevice();
    const GraphicDisplay& gfxDisplay = GraphicCore::GetGraphicDisplay();
    const GraphicConfig& config = GraphicCore::GetGraphicConfig();

    // TEMP: Recreate TLAS and SRV every frame because visual batch might have changed
    InitializeAccelerationStructure(&graphicContext.GetVisualBatch());

    // TEMP: Recreate shader table every frame because descriptors may have been reallocated and thus address changed
    // For many passes, descriptor heap that is bound should have a known layout (except for bindless)
    // In those cases, even if the descriptors are re-allocated, the heap address/table address should remain the same.
    //
    // TODO: For this pass, need to allocate descriptors manually such that shader table can remain the same
    //       However, we may lose global descriptors that are in Graphics::GraphicCore
    //       Maybe some mechanism to copy all the descriptors to a new heap made specifically for this pass needs to be
    //       done? Something like a dynamic descriptor heaps?
    InitializeShaderBindingTable();

    graphicContext.PushMarker("Clear");
    graphicContext.TransitionResource(gfxDisplay.GetBackBuffer(), RhiResourceState::RenderTarget);
    graphicContext.ClearColor(gfxDisplay.GetBackBufferRtv(), config.GetClearColor());
    graphicContext.PopMarker();
    graphicContext.FinalizeAndExecute();
    graphicContext.Reset();

    graphicContext.PushMarker("Raytrace");
    graphicContext.TransitionResource(*m_OutputTexture, RhiResourceState::UnorderedAccess);
    graphicContext.SetComputeRootSignature(*m_GlobalRootSignature);
    graphicContext.SetRaytracingPipelineState(*m_RaytracingPipelineState);
    graphicContext.DispatchRays({ GraphicCore::GetGraphicConfig().GetResolution().x,
                                  GraphicCore::GetGraphicConfig().GetResolution().y,
                                  1,
                                  m_RaytracingShaderBindingTable.get() });
    graphicContext.PopMarker();
    graphicContext.FinalizeAndExecute();
    graphicContext.Reset();

    graphicContext.PushMarker("Copy to render target");
    graphicContext.TransitionResource(*m_OutputTexture, RhiResourceState::CopySrc);
    graphicContext.TransitionResource(gfxDisplay.GetBackBuffer(), RhiResourceState::CopyDest);
    graphicContext.CopyResource(*m_OutputTexture, gfxDisplay.GetBackBuffer());
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

    if (GraphicCore::GetGraphicConfig().GetUseShaderDaemon())
        GraphicCore::GetShaderDaemon().RegisterShader(*m_LibraryShader);
}

void Ether::Graphics::RaytracedGBufferPass::InitializeRootSignatures()
{
    std::unique_ptr<RhiRootSignatureDesc> rsDesc = GraphicCore::GetDevice().CreateRootSignatureDesc(2, 0, true);

    rsDesc->SetAsDescriptorTable(0, 1, RhiShaderVisibility::All);
    rsDesc->SetAsDescriptorTable(1, 1, RhiShaderVisibility::All);

    rsDesc->SetDescriptorTableRange(0, RhiDescriptorType::Uav);
    rsDesc->SetDescriptorTableRange(1, RhiDescriptorType::Srv);

    m_RayGenRootSignature = rsDesc->Compile();

    rsDesc = GraphicCore::GetDevice().CreateRootSignatureDesc(0, 0, true);
    m_HitMissRootSignature = rsDesc->Compile();

    rsDesc = GraphicCore::GetDevice().CreateRootSignatureDesc(0, 0, false);
    m_GlobalRootSignature = rsDesc->Compile();
}

void Ether::Graphics::RaytracedGBufferPass::InitializePipelineStates()
{
    const RhiDevice& gfxDevice = GraphicCore::GetDevice();

    RhiRaytracingPipelineStateDesc desc = {};
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

void Ether::Graphics::RaytracedGBufferPass::InitializeShaderBindingTable()
{
    const RhiDevice& gfxDevice = GraphicCore::GetDevice();

    RhiRaytracingShaderBindingTableDesc desc = {};
    desc.m_MaxRootSignatureSize = 8 + 8; // Raygen's descriptor table1 (8), Raygen's descriptor table2 (8)
    desc.m_RaytracingPipelineState = m_RaytracingPipelineState.get();
    desc.m_HitGroupName = kHitGroup;
    desc.m_MissShaderName = kMissShader;
    desc.m_RayGenShaderName = kRayGenShader;
    desc.m_RayGenRootTableAddress1 = m_OutputTextureUav->GetGpuAddress();
    desc.m_RayGenRootTableAddress2 = m_TlasSrv->GetGpuAddress();

    m_RaytracingShaderBindingTable = gfxDevice.CreateRaytracingShaderBindingTable(desc);
}

void Ether::Graphics::RaytracedGBufferPass::InitializeAccelerationStructure(const VisualBatch* visualBatch)
{
    RhiTopLevelAccelerationStructureDesc desc = {};
    desc.m_VisualBatch = (void*)visualBatch;
    m_TopLevelAccelerationStructure = GraphicCore::GetDevice().CreateAccelerationStructure(desc);

    CommandContext tlasBuildContext(RhiCommandType::Graphic, "TLAS Build Context - Build GBuffer TLAS");
    tlasBuildContext.PushMarker("Build TLAS");
    tlasBuildContext.TransitionResource(
        *m_TopLevelAccelerationStructure->m_ScratchBuffer,
        RhiResourceState::UnorderedAccess);
    tlasBuildContext.BuildTopLevelAccelerationStructure(*m_TopLevelAccelerationStructure);
    tlasBuildContext.PopMarker();
    tlasBuildContext.FinalizeAndExecute(true);
    tlasBuildContext.Reset();

    m_TopLevelAccelerationStructure->m_ScratchBuffer.reset();

    auto srvAlloc = GraphicCore::GetSrvCbvUavAllocator().Allocate();
    RhiShaderResourceViewDesc srvDesc = {};
    srvDesc.m_Dimensions = RhiShaderResourceDimensions::RTAccelerationStructure;
    srvDesc.m_TargetCpuAddress = ((DescriptorAllocation&)(*srvAlloc)).GetCpuAddress();

    m_TlasSrv = GraphicCore::GetDevice().CreateShaderResourceView(srvDesc);
    GraphicCore::GetSrvCbvUavAllocator().Free(std::move(srvAlloc));
}
