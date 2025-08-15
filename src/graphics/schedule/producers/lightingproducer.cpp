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

// TODO: Rename to deferredlightsproducer
#include "lightingproducer.h"

#include "graphics/graphiccore.h"
#include "graphics/rhi/rhiraytracingpipelinestate.h"
#include "graphics/resources/mesh.h"
#include "graphics/resources/material.h"
#include "graphics/shaders/common/raytracingconstants.h"
#include "graphics/shaders/common/globalconstants.h"

DEFINE_GFX_PA(LightingProducer)
DEFINE_GFX_UA(LightingTexture)
DEFINE_GFX_SR(LightingTexture)

// 2 because pathtracer pass uses 1. TODO: Rename better
DEFINE_GFX_SR(RTGeometryInfo2)
DEFINE_GFX_AS(RTTopLevelAccelerationStructure2)

DECLARE_GFX_SR(GBufferTexture0)
DECLARE_GFX_SR(GBufferTexture1)
DECLARE_GFX_SR(GBufferTexture2)
DECLARE_GFX_SR(GBufferTexture3)
DECLARE_GFX_CB(GlobalRingBuffer)
DECLARE_GFX_SR(MaterialTable)

DEFINE_GFX_UA(GIReservoir_Initial)
DEFINE_GFX_UA(GIReservoir_History)
DEFINE_GFX_UA(GIReservoir_Staging)

static const wchar_t* k_RayGenShader = L"RayGeneration";
static const wchar_t* k_MissShader = L"Miss";
static const wchar_t* k_ClosestHitShader = L"ClosestHit";
static const wchar_t* k_HitGroupName = L"HitGroup";
static const wchar_t* s_EntryPoints[] = { k_RayGenShader, k_MissShader, k_ClosestHitShader };

Ether::Graphics::LightingProducer::LightingProducer()
    : GraphicProducer("LightingProducer")
{
}

void Ether::Graphics::LightingProducer::Initialize(ResourceContext& rc)
{
    CreateShaders();
    CreateRootSignature();
    CreatePipelineState(rc);
}

void Ether::Graphics::LightingProducer::GetInputOutput(ScheduleContext& schedule, ResourceContext& rc)
{
    ethVector2u resolution = GraphicCore::GetGraphicConfig().GetResolution();
    uint32_t numVisuals = GraphicCore::GetGraphicRenderer().GetRenderData().m_Visuals.size();

    schedule.NewUA(ACCESS_GFX_UA(LightingTexture), resolution.x, resolution.y, BackBufferHdrFormat, RhiResourceDimension::Texture2D);
    schedule.NewSR(ACCESS_GFX_SR(LightingTexture), resolution.x, resolution.y, BackBufferHdrFormat, RhiResourceDimension::Texture2D);
    schedule.NewSR(ACCESS_GFX_SR(RTGeometryInfo2), sizeof(Shader::GeometryInfo) * numVisuals, 0, RhiFormat::Unknown, RhiResourceDimension::StructuredBuffer, sizeof(Shader::GeometryInfo));
    schedule.NewAS(ACCESS_GFX_AS(RTTopLevelAccelerationStructure2), GraphicCore::GetGraphicRenderer().GetRenderData().m_Visuals);

    schedule.Read(ACCESS_GFX_SR(GBufferTexture0));
    schedule.Read(ACCESS_GFX_SR(GBufferTexture1));
    schedule.Read(ACCESS_GFX_SR(GBufferTexture2));
    schedule.Read(ACCESS_GFX_SR(GBufferTexture3));
    schedule.Read(ACCESS_GFX_CB(GlobalRingBuffer));
    schedule.Read(ACCESS_GFX_SR(MaterialTable));

    /* ReSTIR GI Implementation */
    const uint32_t reservoirSize = resolution.x * resolution.y;
    schedule.NewUA(ACCESS_GFX_UA(GIReservoir_Initial), sizeof(Shader::GIPackedReservoir) * reservoirSize, 0, RhiFormat::Unknown, RhiResourceDimension::StructuredBuffer, sizeof(Shader::GIPackedReservoir));
    schedule.NewUA(ACCESS_GFX_UA(GIReservoir_History), sizeof(Shader::GIPackedReservoir) * reservoirSize, 0, RhiFormat::Unknown, RhiResourceDimension::StructuredBuffer, sizeof(Shader::GIPackedReservoir));
    schedule.NewUA(ACCESS_GFX_UA(GIReservoir_Staging), sizeof(Shader::GIPackedReservoir) * reservoirSize, 0, RhiFormat::Unknown, RhiResourceDimension::StructuredBuffer, sizeof(Shader::GIPackedReservoir));

    InitializeShaderBindingTable(rc);
}

void Ether::Graphics::LightingProducer::RenderFrame(GraphicContext& ctx, ResourceContext& rc)
{
    const RhiDevice& gfxDevice = GraphicCore::GetDevice();
    const GraphicDisplay& gfxDisplay = GraphicCore::GetGraphicDisplay();
    const GraphicConfig& config = GraphicCore::GetGraphicConfig();
    const std::vector<Visual>& visuals = GraphicCore::GetGraphicRenderer().GetRenderData().m_Visuals;

    const auto resolution = GraphicCore::GetGraphicConfig().GetResolution();
    ctx.PushMarker("Render Direct (temp) & Indirect Lighting");

    auto alloc = GetFrameAllocator().Allocate({ sizeof(Shader::GeometryInfo) * visuals.size(), 256 });
    uint64_t ringBufferOffset = gfxDisplay.GetBackBufferIndex() * AlignUp(sizeof(Shader::GlobalConstants), 256);
    Shader::GeometryInfo* geometryInfos = (Shader::GeometryInfo*)alloc->GetCpuHandle();

    for (uint32_t i = 0; i < visuals.size(); ++i)
    {
        geometryInfos[i].m_VBDescriptorIndex = visuals[i].m_Mesh->GetVertexBufferSrvIndex();
        geometryInfos[i].m_IBDescriptorIndex = visuals[i].m_Mesh->GetIndexBufferSrvIndex();
        geometryInfos[i].m_MaterialIndex = visuals[i].m_Material->GetTransientMaterialIdx();
    }
    ctx.CopyBufferRegion(dynamic_cast<UploadBufferAllocation&>(*alloc).GetResource(), *rc.GetResource(ACCESS_GFX_SR(RTGeometryInfo2)), sizeof(Shader::GeometryInfo) * visuals.size(), 0, 0);
    ctx.SetSrvCbvUavDescriptorHeap(GraphicCore::GetSrvCbvUavAllocator().GetDescriptorHeap());
    ctx.SetSamplerDescriptorHeap(GraphicCore::GetSamplerAllocator().GetDescriptorHeap());
    ctx.SetComputeRootSignature(*m_RootSignature);
    ctx.SetComputeRootConstantBufferView(0, rc.GetResource(ACCESS_GFX_CB(GlobalRingBuffer))->GetGpuAddress() + ringBufferOffset);
    ctx.SetComputeRootShaderResourceView(1, rc.GetResource(ACCESS_GFX_AS(RTTopLevelAccelerationStructure2))->GetGpuAddress());
    ctx.SetComputeRootShaderResourceView(2, rc.GetResource(ACCESS_GFX_SR(RTGeometryInfo2))->GetGpuAddress());
    ctx.SetComputeRootShaderResourceView(3, rc.GetResource(ACCESS_GFX_SR(MaterialTable))->GetGpuAddress());
    ctx.SetComputeRootDescriptorTable(4, ACCESS_GFX_SR(GBufferTexture0)->GetGpuAddress());
    ctx.SetComputeRootDescriptorTable(5, ACCESS_GFX_SR(GBufferTexture1)->GetGpuAddress());
    ctx.SetComputeRootDescriptorTable(6, ACCESS_GFX_SR(GBufferTexture2)->GetGpuAddress());
    ctx.SetComputeRootDescriptorTable(7, ACCESS_GFX_SR(GBufferTexture3)->GetGpuAddress());

    const bool temporalResampling = config.m_ReSTIRGIConfig.m_TemporalResampling;
    const bool spatialResampling = config.m_ReSTIRGIConfig.m_SpatialResampling;
    const bool spatialFeedback = config.m_ReSTIRGIConfig.m_SpatialFeedback;

    auto initialReservoir = ACCESS_GFX_UA(GIReservoir_Initial);
    auto historyReservoir = ACCESS_GFX_UA(GIReservoir_History);
    auto stagingReservoir = ACCESS_GFX_UA(GIReservoir_Staging);
    auto finalReservoir = initialReservoir;

    if (spatialResampling && temporalResampling)
    {
        finalReservoir = historyReservoir;
    }
    else if (spatialResampling || temporalResampling)
    {
        if (temporalResampling && GraphicCore::GetGraphicRenderer().GetFrameNumber() % 2 == 0)
        {
            historyReservoir = ACCESS_GFX_UA(GIReservoir_Staging);
            stagingReservoir = ACCESS_GFX_UA(GIReservoir_History);
        }

        finalReservoir = stagingReservoir;
    }

    // Initial Reservoir Generation
    {
        ctx.PushMarker("ReSTIR - Initial Reservoir Generation");
        ctx.SetRaytracingShaderBindingTable(m_InitialGenerationSBT);
        ctx.SetRaytracingPipelineState((RhiRaytracingPipelineState&)rc.GetPipelineState(*m_InitialGenerationPsoDesc));
        ctx.SetComputeRootDescriptorTable(10, initialReservoir->GetGpuAddress());
        ctx.DispatchRays(resolution.x, resolution.y, 1);
        ctx.PopMarker();
    }

    // Temporal Resampling Pass
    if (temporalResampling)
    {
        ctx.PushMarker("ReSTIR - Temporal Resampling");
        ctx.InsertUavBarrier(*rc.GetResource(ACCESS_GFX_UA(GIReservoir_Initial)));
        ctx.SetComputePipelineState((RhiComputePipelineState&)rc.GetPipelineState(*m_TemporalResamplingPsoDesc));
        ctx.SetComputeRootDescriptorTable(8, initialReservoir->GetGpuAddress());
        ctx.SetComputeRootDescriptorTable(9, historyReservoir->GetGpuAddress());
        ctx.SetComputeRootDescriptorTable(10, stagingReservoir->GetGpuAddress());
        ctx.Dispatch(std::ceil(resolution.x / 32.0), std::ceil(resolution.y / 32.0), 1);
        ctx.PopMarker();
    }

    // Final shading pass
    {
        ctx.PushMarker("ReSTIR - Final Lighting Evaluation");
        ctx.InsertUavBarrier(*rc.GetResource(finalReservoir));
        ctx.SetRaytracingShaderBindingTable(m_LightingEvaluationSBT);
        ctx.SetRaytracingPipelineState((RhiRaytracingPipelineState&)rc.GetPipelineState(*m_LightingEvaluationPsoDesc));
        ctx.SetComputeRootDescriptorTable(8, finalReservoir->GetGpuAddress());
        ctx.SetComputeRootDescriptorTable(11, ACCESS_GFX_UA(LightingTexture)->GetGpuAddress());
        ctx.DispatchRays(resolution.x, resolution.y, 1);
        ctx.PopMarker();
    }

    ctx.PopMarker();
}

bool Ether::Graphics::LightingProducer::IsEnabled()
{
    if (!GraphicCore::GetGraphicConfig().m_IsRaytracingEnabled)
        return false;

    if (GraphicCore::GetGraphicRenderer().GetRenderData().m_Visuals.empty())
        return false;

    return true;
}

void Ether::Graphics::LightingProducer::CreateShaders()
{
    const RhiDevice& gfxDevice = GraphicCore::GetDevice();
    m_InitialGenerationShader = gfxDevice.CreateShader({ "lighting\\restir\\restirgi_initialgeneration.hlsl", "", RhiShaderType::Library });
    m_TemporalResamplingShader = gfxDevice.CreateShader({ "lighting\\restir\\restirgi_temporalresampling.hlsl", "CS_Main", RhiShaderType::Compute });
    m_LightingEvaluationShader = gfxDevice.CreateShader({ "lighting\\restir\\restirgi_shadereservoir.hlsl", "", RhiShaderType::Library });

    // Manually compile shader since raytracing PSO caching has not been implemented yet
    m_InitialGenerationShader->Compile();
    m_TemporalResamplingShader->Compile();
    m_LightingEvaluationShader->Compile();

    GraphicCore::GetShaderDaemon().RegisterShader(*m_InitialGenerationShader);
    GraphicCore::GetShaderDaemon().RegisterShader(*m_TemporalResamplingShader);
    GraphicCore::GetShaderDaemon().RegisterShader(*m_LightingEvaluationShader);
}

void Ether::Graphics::LightingProducer::CreateRootSignature()
{
    std::unique_ptr<RhiRootSignatureDesc> raygenRsDesc = GraphicCore::GetDevice().CreateRootSignatureDesc(12, 0);
    raygenRsDesc->SetAsConstantBufferView(0, 0, RhiShaderVisibility::All);        // (b0) Global Constants    
    raygenRsDesc->SetAsShaderResourceView(1, 0, RhiShaderVisibility::All);        // (t0) TLAS
    raygenRsDesc->SetAsShaderResourceView(2, 1, RhiShaderVisibility::All);        // (t1) RTGeometryInfo2
    raygenRsDesc->SetAsShaderResourceView(3, 2, RhiShaderVisibility::All);        // (t2) MaterialTable
    raygenRsDesc->SetAsDescriptorTable(4, 1, RhiShaderVisibility::All);
    raygenRsDesc->SetDescriptorTableRange(4, RhiDescriptorType::Srv, 1, 0, 3);    // (t3) GBuffer0
    raygenRsDesc->SetAsDescriptorTable(5, 1, RhiShaderVisibility::All);      
    raygenRsDesc->SetDescriptorTableRange(5, RhiDescriptorType::Srv, 1, 0, 4);    // (t4) GBuffer1
    raygenRsDesc->SetAsDescriptorTable(6, 1, RhiShaderVisibility::All);
    raygenRsDesc->SetDescriptorTableRange(6, RhiDescriptorType::Srv, 1, 0, 5);    // (t5) GBuffer2
    raygenRsDesc->SetAsDescriptorTable(7, 1, RhiShaderVisibility::All);
    raygenRsDesc->SetDescriptorTableRange(7, RhiDescriptorType::Srv, 1, 0, 6);    // (t6) GBuffer3
    raygenRsDesc->SetAsDescriptorTable(8, 1, RhiShaderVisibility::All);
    raygenRsDesc->SetDescriptorTableRange(8, RhiDescriptorType::Uav, 1, 0, 0);    // (u0) GIReservoir_Input
    raygenRsDesc->SetAsDescriptorTable(9, 1, RhiShaderVisibility::All);
    raygenRsDesc->SetDescriptorTableRange(9, RhiDescriptorType::Uav, 1, 0, 1);    // (u1) GIReservoir_History
    raygenRsDesc->SetAsDescriptorTable(10, 1, RhiShaderVisibility::All);
    raygenRsDesc->SetDescriptorTableRange(10, RhiDescriptorType::Uav, 1, 0, 2);   // (u2) GIReservoir_Output
    raygenRsDesc->SetAsDescriptorTable(11, 1, RhiShaderVisibility::All);
    raygenRsDesc->SetDescriptorTableRange(11, RhiDescriptorType::Uav, 1, 0, 3);   // (u3) LightingOutput
    raygenRsDesc->SetFlags(RhiRootSignatureFlag::DirectlyIndexed);
    m_RootSignature = raygenRsDesc->Compile((GetName() + " Root Signature (RayGen)").c_str());
}

void Ether::Graphics::LightingProducer::CreatePipelineState(ResourceContext& rc)
{
    uint32_t numExports = sizeof(s_EntryPoints) / sizeof(s_EntryPoints[0]);
    m_InitialGenerationPsoDesc = GraphicCore::GetDevice().CreateRaytracingPipelineStateDesc();
    m_InitialGenerationPsoDesc->SetLibraryShader(*m_InitialGenerationShader);
    m_InitialGenerationPsoDesc->SetHitGroupName(k_HitGroupName);
    m_InitialGenerationPsoDesc->SetClosestHitShaderName(k_ClosestHitShader);
    m_InitialGenerationPsoDesc->SetMissShaderName(k_MissShader);
    m_InitialGenerationPsoDesc->SetRayGenShaderName(k_RayGenShader);
    m_InitialGenerationPsoDesc->SetMaxRecursionDepth(4);
    m_InitialGenerationPsoDesc->SetMaxAttributeSize(sizeof(float) * 2); // from built in attributes
    m_InitialGenerationPsoDesc->SetMaxPayloadSize(sizeof(Shader::RayPayload) + 4);
    m_InitialGenerationPsoDesc->SetRootSignature(*m_RootSignature);
    m_InitialGenerationPsoDesc->PushLibrary(s_EntryPoints, numExports);
    m_InitialGenerationPsoDesc->PushHitProgram();
    m_InitialGenerationPsoDesc->PushShaderConfig();
    m_InitialGenerationPsoDesc->PushExportAssociation(s_EntryPoints, numExports);
    m_InitialGenerationPsoDesc->PushPipelineConfig();
    m_InitialGenerationPsoDesc->PushGlobalRootSignature();
    rc.RegisterPipelineState((GetName() + " Initial Generation Raytracing Pipeline State").c_str(), *m_InitialGenerationPsoDesc);

    m_LightingEvaluationPsoDesc = GraphicCore::GetDevice().CreateRaytracingPipelineStateDesc();
    m_LightingEvaluationPsoDesc->SetLibraryShader(*m_LightingEvaluationShader);
    m_LightingEvaluationPsoDesc->SetHitGroupName(k_HitGroupName);
    m_LightingEvaluationPsoDesc->SetClosestHitShaderName(k_ClosestHitShader);
    m_LightingEvaluationPsoDesc->SetMissShaderName(k_MissShader);
    m_LightingEvaluationPsoDesc->SetRayGenShaderName(k_RayGenShader);
    m_LightingEvaluationPsoDesc->SetMaxRecursionDepth(1);
    m_LightingEvaluationPsoDesc->SetMaxAttributeSize(sizeof(float) * 2); // from built in attributes
    m_LightingEvaluationPsoDesc->SetMaxPayloadSize(sizeof(Shader::RayPayload) + 4);
    m_LightingEvaluationPsoDesc->SetRootSignature(*m_RootSignature);
    m_LightingEvaluationPsoDesc->PushLibrary(s_EntryPoints, numExports);
    m_LightingEvaluationPsoDesc->PushHitProgram();
    m_LightingEvaluationPsoDesc->PushShaderConfig();
    m_LightingEvaluationPsoDesc->PushExportAssociation(s_EntryPoints, numExports);
    m_LightingEvaluationPsoDesc->PushPipelineConfig();
    m_LightingEvaluationPsoDesc->PushGlobalRootSignature();
    rc.RegisterPipelineState((GetName() + " Lighting Evaluation Raytracing Pipeline State").c_str(), *m_LightingEvaluationPsoDesc);

    m_TemporalResamplingPsoDesc = GraphicCore::GetDevice().CreateComputePipelineStateDesc();
    m_TemporalResamplingPsoDesc->SetComputeShader(*m_TemporalResamplingShader);
    m_TemporalResamplingPsoDesc->SetRootSignature(*m_RootSignature);
    rc.RegisterPipelineState((GetName() + " Temporal Resampling Pipeline State").c_str(), *m_TemporalResamplingPsoDesc);
}

void Ether::Graphics::LightingProducer::InitializeShaderBindingTable(ResourceContext& rc)
{
    const GraphicDisplay& gfxDisplay = GraphicCore::GetGraphicDisplay();

    RhiRaytracingShaderBindingTableDesc desc = {};
    desc.m_MaxRootSignatureSize = 0; // Only ever use global root signature since we are bindless
    desc.m_RaytracingPipelineState = &(RhiRaytracingPipelineState&)rc.GetPipelineState(*m_InitialGenerationPsoDesc);
    desc.m_HitGroupName = k_HitGroupName;
    desc.m_MissShaderName = k_MissShader;
    desc.m_RayGenShaderName = k_RayGenShader;
    m_InitialGenerationSBT = &rc.CreateRaytracingShaderBindingTable("RT Bindings Table (Initial Generation)", desc);

    desc.m_RaytracingPipelineState = &(RhiRaytracingPipelineState&)rc.GetPipelineState(*m_LightingEvaluationPsoDesc);
    m_LightingEvaluationSBT = &rc.CreateRaytracingShaderBindingTable("RT Bindings Table (Lighting Evaluation)", desc);
}
