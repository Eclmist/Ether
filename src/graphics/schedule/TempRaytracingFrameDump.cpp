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

static const wchar_t* kRayGenShader = L"RayGeneration";
static const wchar_t* kMissShader = L"Miss";
static const wchar_t* kClosestHitShader = L"ClosestHit";
static const wchar_t* kHitGroup = L"HitGroup";
static const wchar_t* EntryPoints[] = { kRayGenShader, kMissShader, kClosestHitShader };

void Ether::Graphics::TempRaytracingFrameDump::Initialize(ResourceContext& resourceContext)
{
    InitializeShaders();
    InitializeRootSignatures();
    InitializePipelineStates();

    RhiCommitedResourceDesc globalConstantsCbvResource = {};
    globalConstantsCbvResource.m_Name = "RaytracedGBuffer::GlobalConstants";
    globalConstantsCbvResource.m_HeapType = RhiHeapType::Upload;
    globalConstantsCbvResource.m_State = RhiResourceState::Common;
    globalConstantsCbvResource.m_ResourceDesc = RhiCreateBufferResourceDesc(sizeof(Shader::GlobalConstants));
    m_ConstantBuffer = GraphicCore::GetDevice().CreateCommittedResource(globalConstantsCbvResource);

    RhiConstantBufferViewDesc cbvDesc = {};
    cbvDesc.m_Resource = m_ConstantBuffer.get();
    cbvDesc.m_TargetCpuAddress = ((DescriptorAllocation&)(*m_RootTableDescriptorAlloc)).GetCpuAddress(2);
    cbvDesc.m_TargetGpuAddress = ((DescriptorAllocation&)(*m_RootTableDescriptorAlloc)).GetGpuAddress(2);
    cbvDesc.m_BufferSize = sizeof(Shader::GlobalConstants);
    m_ConstantBufferView = GraphicCore::GetDevice().CreateConstantBufferView(cbvDesc);
}

void Ether::Graphics::TempRaytracingFrameDump::FrameSetup(ResourceContext& resourceContext)
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

    RhiUnorderedAccessViewDesc uavDesc = {};
    uavDesc.m_Format = RhiFormat::R8G8B8A8Unorm;
    uavDesc.m_Resource = m_OutputTexture.get();
    uavDesc.m_TargetCpuAddress = ((DescriptorAllocation&)(*m_RootTableDescriptorAlloc)).GetCpuAddress(1);
    uavDesc.m_TargetGpuAddress = ((DescriptorAllocation&)(*m_RootTableDescriptorAlloc)).GetGpuAddress(1);
    uavDesc.m_Dimensions = RhiUnorderedAccessDimension::Texture2D;

    m_OutputTextureUav = GraphicCore::GetDevice().CreateUnorderedAccessView(uavDesc);
}

void Ether::Graphics::TempRaytracingFrameDump::Render(GraphicContext& graphicContext, ResourceContext& resourceContext)
{
    ETH_MARKER_EVENT("Raytraced GBuffer Pass - Render");
    const RhiDevice& gfxDevice = GraphicCore::GetDevice();
    const GraphicDisplay& gfxDisplay = GraphicCore::GetGraphicDisplay();
    const GraphicConfig& config = GraphicCore::GetGraphicConfig();

    if (graphicContext.GetVisualBatch().m_Visuals.empty())
        return;

    static bool isTlasInitialized = false;

    if (!isTlasInitialized)
    {
        // TEMP: Recreate TLAS and SRV every frame because visual batch might have changed
        InitializeAccelerationStructure(&graphicContext.GetVisualBatch(), graphicContext);

        // TEMP: Recreate shader table every frame because descriptors may have been reallocated and thus address
        // changed For many passes, descriptor heap that is bound should have a known layout (except for bindless) In
        // those cases, even if the descriptors are re-allocated, the heap address/table address should remain the same.
        //
        // TODO: For this pass, need to allocate descriptors manually such that shader table can remain the same
        //       However, we may lose global descriptors that are in Graphics::GraphicCore
        //       Maybe some mechanism to copy all the descriptors to a new heap made specifically for this pass needs to
        //       be done? Something like a dynamic descriptor heaps?
        InitializeShaderBindingTable();

        isTlasInitialized = true;
    }

    const auto resolution = GraphicCore::GetGraphicConfig().GetResolution();

    graphicContext.PushMarker("Clear");
    graphicContext.TransitionResource(gfxDisplay.GetBackBuffer(), RhiResourceState::RenderTarget);
    graphicContext.ClearColor(gfxDisplay.GetBackBufferRtv(), config.GetClearColor());
    graphicContext.PopMarker();

    graphicContext.PushMarker("Raytrace");
    graphicContext.TransitionResource(*m_OutputTexture, RhiResourceState::UnorderedAccess);
    graphicContext.SetComputeRootSignature(*m_GlobalRootSignature);
    graphicContext.SetDescriptorHeap(GraphicCore::GetSrvCbvUavAllocator().GetDescriptorHeap());
    graphicContext.SetRaytracingShaderBindingTable(*m_RaytracingShaderBindingTable);
    graphicContext.SetRaytracingPipelineState(*m_RaytracingPipelineState);
    graphicContext.DispatchRays(resolution.x, resolution.y, 1);
    graphicContext.PopMarker();

    graphicContext.PushMarker("Copy to render target");
    graphicContext.TransitionResource(*m_OutputTexture, RhiResourceState::CopySrc);
    graphicContext.TransitionResource(gfxDisplay.GetBackBuffer(), RhiResourceState::CopyDest);
    graphicContext.CopyResource(*m_OutputTexture, gfxDisplay.GetBackBuffer());
    graphicContext.PopMarker();
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
    rsDesc->SetDescriptorTableRange(0, RhiDescriptorType::Srv, 1, 0);
    rsDesc->SetDescriptorTableRange(0, RhiDescriptorType::Uav, 1, 1);
    m_RayGenRootSignature = rsDesc->Compile("Ray Generation Root Signature (Local)");

    rsDesc = GraphicCore::GetDevice().CreateRootSignatureDesc(0, 0, true);
    m_HitMissRootSignature = rsDesc->Compile("Empty Root Signature (Local)");

    rsDesc = GraphicCore::GetDevice().CreateRootSignatureDesc(1, 0, false);
    rsDesc->SetAsConstantBufferView(0, 0, RhiShaderVisibility::All);
    m_GlobalRootSignature = rsDesc->Compile("Empty Root Signature (Global)");

    m_RootTableDescriptorAlloc = GraphicCore::GetSrvCbvUavAllocator().Allocate(3);
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
    desc.m_MaxPayloadSize = sizeof(ethVector3);  // From payload struct in shader
    desc.m_MaxRecursionDepth = 1;
    desc.m_LibraryShaderDesc = { m_Shader.get(), EntryPoints, sizeof(EntryPoints) / sizeof(EntryPoints[0]) };
    desc.m_RayGenRootSignature = m_RayGenRootSignature.get();
    desc.m_HitMissRootSignature = m_HitMissRootSignature.get();
    desc.m_GlobalRootSignature = m_GlobalRootSignature.get();

    m_RaytracingPipelineState = gfxDevice.CreateRaytracingPipelineState(desc);
}

void Ether::Graphics::TempRaytracingFrameDump::InitializeShaderBindingTable()
{
    const RhiDevice& gfxDevice = GraphicCore::GetDevice();

    RhiRaytracingShaderBindingTableDesc desc = {};
    desc.m_MaxRootSignatureSize = 8; // Raygen's descriptor table1 (8)
    desc.m_RaytracingPipelineState = m_RaytracingPipelineState.get();
    desc.m_HitGroupName = kHitGroup;
    desc.m_MissShaderName = kMissShader;
    desc.m_RayGenShaderName = kRayGenShader;
    desc.m_RayGenRootTableAddress = m_TlasSrv->GetGpuAddress();
    m_RaytracingShaderBindingTable = gfxDevice.CreateRaytracingShaderBindingTable(desc);
}

void Ether::Graphics::TempRaytracingFrameDump::InitializeAccelerationStructure(
    const VisualBatch* visualBatch,
    GraphicContext& context)
{
    RhiTopLevelAccelerationStructureDesc desc = {};
    desc.m_VisualBatch = (void*)visualBatch;
    m_TopLevelAccelerationStructure = GraphicCore::GetDevice().CreateAccelerationStructure(desc);

    context.PushMarker("Build GBuffer TLAS");
    context.TransitionResource(*m_TopLevelAccelerationStructure->m_ScratchBuffer, RhiResourceState::UnorderedAccess);
    context.BuildTopLevelAccelerationStructure(*m_TopLevelAccelerationStructure);
    context.PopMarker();

    RhiShaderResourceViewDesc srvDesc = {};
    srvDesc.m_Dimensions = RhiShaderResourceDimension::RTAccelerationStructure;
    srvDesc.m_TargetCpuAddress = ((DescriptorAllocation&)(*m_RootTableDescriptorAlloc)).GetCpuAddress(0);
    srvDesc.m_TargetGpuAddress = ((DescriptorAllocation&)(*m_RootTableDescriptorAlloc)).GetGpuAddress(0);
    srvDesc.m_RaytracingAccelerationStructureAddress = m_TopLevelAccelerationStructure->m_DataBuffer->GetGpuAddress();
    srvDesc.m_Resource = m_TopLevelAccelerationStructure->m_DataBuffer.get();
    m_TlasSrv = GraphicCore::GetDevice().CreateShaderResourceView(srvDesc);
}

