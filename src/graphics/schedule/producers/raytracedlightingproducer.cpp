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

#include "raytracedlightingproducer.h"

#include "graphics/graphiccore.h"
#include "graphics/rhi/rhiraytracingpipelinestate.h"
#include "graphics/resources/mesh.h"
#include "graphics/resources/material.h"
#include "graphics/shaders/common/raytracingconstants.h"
#include "graphics/shaders/common/globalconstants.h"

DEFINE_GFX_PA(RaytracedLightingProducer)
DEFINE_GFX_UA(RTLightingTexture)
DEFINE_GFX_SR(RTLightingTexture)
DEFINE_GFX_UA(RTIndirectTexture)
DEFINE_GFX_SR(RTIndirectTexture)
DEFINE_GFX_SR(RTGeometryInfo)
DEFINE_GFX_AS(RTTopLevelAccelerationStructure)

DECLARE_GFX_SR(GBufferTexture0)
DECLARE_GFX_SR(GBufferTexture1)
DECLARE_GFX_SR(GBufferTexture2)
DECLARE_GFX_CB(GlobalRingBuffer)
DECLARE_GFX_SR(MaterialTable)

static const wchar_t* k_RayGenShader = L"RayGeneration";
static const wchar_t* k_MissShader = L"Miss";
static const wchar_t* k_ClosestHitShader = L"ClosestHit";
static const wchar_t* k_HitGroupName = L"HitGroup";
static const wchar_t* s_EntryPoints[] = { k_RayGenShader, k_MissShader, k_ClosestHitShader };

Ether::Graphics::RaytracedLightingProducer::RaytracedLightingProducer()
    : GraphicProducer("RaytracedLightingProducer")
{
}

void Ether::Graphics::RaytracedLightingProducer::Initialize(ResourceContext& rc)
{
    CreateShaders();
    CreateRootSignature();
    CreatePipelineState(rc);
}

void Ether::Graphics::RaytracedLightingProducer::GetInputOutput(ScheduleContext& schedule, ResourceContext& rc)
{
    ethVector2u resolution = GraphicCore::GetGraphicConfig().GetResolution();
    uint32_t numVisuals = GraphicCore::GetGraphicRenderer().GetRenderData().m_Visuals.size();

    schedule.NewUA(ACCESS_GFX_UA(RTLightingTexture), resolution.x, resolution.y, RhiFormat::R11G11B10Float, RhiResourceDimension::Texture2D);
    schedule.NewSR(ACCESS_GFX_SR(RTLightingTexture), resolution.x, resolution.y, RhiFormat::R11G11B10Float, RhiResourceDimension::Texture2D);
    schedule.NewUA(ACCESS_GFX_UA(RTIndirectTexture), resolution.x, resolution.y, RhiFormat::R11G11B10Float, RhiResourceDimension::Texture2D);
    schedule.NewSR(ACCESS_GFX_SR(RTIndirectTexture), resolution.x, resolution.y, RhiFormat::R11G11B10Float, RhiResourceDimension::Texture2D);
    schedule.NewSR(ACCESS_GFX_SR(RTGeometryInfo), sizeof(Shader::GeometryInfo) * numVisuals, 0, RhiFormat::Unknown, RhiResourceDimension::StructuredBuffer, sizeof(Shader::GeometryInfo));
    schedule.NewAS(ACCESS_GFX_AS(RTTopLevelAccelerationStructure), GraphicCore::GetGraphicRenderer().GetRenderData().m_Visuals);

    schedule.Read(ACCESS_GFX_SR(GBufferTexture0));
    schedule.Read(ACCESS_GFX_SR(GBufferTexture1));
    schedule.Read(ACCESS_GFX_SR(GBufferTexture2));
    schedule.Read(ACCESS_GFX_CB(GlobalRingBuffer));
    schedule.Read(ACCESS_GFX_SR(MaterialTable));

    InitializeShaderBindingTable(rc);
}

void Ether::Graphics::RaytracedLightingProducer::RenderFrame(GraphicContext& ctx, ResourceContext& rc)
{
    const RhiDevice& gfxDevice = GraphicCore::GetDevice();
    const GraphicDisplay& gfxDisplay = GraphicCore::GetGraphicDisplay();
    const GraphicConfig& config = GraphicCore::GetGraphicConfig();
    const std::vector<Visual>& visuals = GraphicCore::GetGraphicRenderer().GetRenderData().m_Visuals;

    const auto resolution = GraphicCore::GetGraphicConfig().GetResolution();
    ctx.PushMarker("Raytrace");
    ctx.TransitionResource(*rc.GetResource(ACCESS_GFX_UA(RTLightingTexture)), RhiResourceState::UnorderedAccess);
    ctx.SetSrvCbvUavDescriptorHeap(GraphicCore::GetSrvCbvUavAllocator().GetDescriptorHeap());
    ctx.SetSamplerDescriptorHeap(GraphicCore::GetSamplerAllocator().GetDescriptorHeap());
    ctx.SetComputeRootSignature(*m_GlobalRootSignature);

    uint64_t ringBufferOffset = gfxDisplay.GetBackBufferIndex() * sizeof(Shader::GlobalConstants);
    ctx.SetComputeRootConstantBufferView(0, rc.GetResource(ACCESS_GFX_CB(GlobalRingBuffer))->GetGpuAddress() + ringBufferOffset);
    ctx.SetComputeRootShaderResourceView(1, rc.GetResource(ACCESS_GFX_AS(RTTopLevelAccelerationStructure))->GetGpuAddress());
    ctx.SetComputeRootShaderResourceView(2, rc.GetResource(ACCESS_GFX_SR(RTGeometryInfo))->GetGpuAddress());
    ctx.SetComputeRootShaderResourceView(3, rc.GetResource(ACCESS_GFX_SR(MaterialTable))->GetGpuAddress());
    ctx.SetComputeRootDescriptorTable(4, ACCESS_GFX_SR(RTIndirectTexture)->GetGpuAddress());
    ctx.SetComputeRootDescriptorTable(5, ACCESS_GFX_SR(GBufferTexture0)->GetGpuAddress());
    ctx.SetComputeRootDescriptorTable(6, ACCESS_GFX_SR(GBufferTexture1)->GetGpuAddress());
    ctx.SetComputeRootDescriptorTable(7, ACCESS_GFX_SR(GBufferTexture2)->GetGpuAddress());
    ctx.SetComputeRootDescriptorTable(8, ACCESS_GFX_UA(RTLightingTexture)->GetGpuAddress());
    ctx.SetComputeRootDescriptorTable(9, ACCESS_GFX_UA(RTIndirectTexture)->GetGpuAddress());
    ctx.SetRaytracingShaderBindingTable(m_RaytracingShaderBindingTable);
    ctx.SetRaytracingPipelineState((RhiRaytracingPipelineState&)rc.GetPipelineState(*m_RTPsoDesc));

    auto alloc = GetFrameAllocator().Allocate({ sizeof(Shader::GeometryInfo) * visuals.size(), 256 });
    Shader::GeometryInfo* geometryInfos = (Shader::GeometryInfo*)alloc->GetCpuHandle();
    for (uint32_t i = 0; i < visuals.size(); ++i)
    {
        geometryInfos[i].m_VBDescriptorIndex = visuals[i].m_Mesh->GetVertexBufferSrvIndex();
        geometryInfos[i].m_IBDescriptorIndex = visuals[i].m_Mesh->GetIndexBufferSrvIndex();
        geometryInfos[i].m_MaterialIndex = visuals[i].m_Material->GetTransientMaterialIdx();
    }

    ctx.CopyBufferRegion(
        dynamic_cast<UploadBufferAllocation&>(*alloc).GetResource(),
        *rc.GetResource(ACCESS_GFX_SR(RTGeometryInfo)),
        sizeof(Shader::GeometryInfo) * visuals.size(),
        0,
        0);

    ctx.DispatchRays(resolution.x, resolution.y, 1);

    //ctx.TransitionResource(*rc.GetResource(ACCESS_GFX_UA(RTIndirectTexture)), RhiResourceState::CopySrc);
    //ctx.TransitionResource(*rc.GetResource(ACCESS_GFX_SR(RTAccumulationTexture)), RhiResourceState::CopyDest);
    //ctx.CopyResource(*rc.GetResource(ACCESS_GFX_UA(RTIndirectTexture)), *rc.GetResource(ACCESS_GFX_SR(RTAccumulationTexture)));

    ctx.PopMarker();
}

bool Ether::Graphics::RaytracedLightingProducer::IsEnabled()
{
    if (!GraphicCore::GetGraphicConfig().m_IsRaytracingEnabled)
        return false;

    if (GraphicCore::GetGraphicRenderer().GetRenderData().m_Visuals.empty())
        return false;

    return true;
}

void Ether::Graphics::RaytracedLightingProducer::CreateShaders()
{
    const RhiDevice& gfxDevice = GraphicCore::GetDevice();
    m_Shader = gfxDevice.CreateShader({ "lighting\\raytracedlights.hlsl", "", RhiShaderType::Library });
    // Manually compile shader since raytracing PSO caching has not been implemented yet
    m_Shader->Compile();

    GraphicCore::GetShaderDaemon().RegisterShader(*m_Shader);
}

void Ether::Graphics::RaytracedLightingProducer::CreateRootSignature()
{
    std::unique_ptr<RhiRootSignatureDesc> rsDesc = GraphicCore::GetDevice().CreateRootSignatureDesc(10, 0);
    rsDesc->SetAsConstantBufferView(0, 0, RhiShaderVisibility::All);        // (b0) Global Constants    
    rsDesc->SetAsShaderResourceView(1, 0, RhiShaderVisibility::All);        // (t0) TLAS
    rsDesc->SetAsShaderResourceView(2, 1, RhiShaderVisibility::All);        // (t1) RTGeometryInfo
    rsDesc->SetAsShaderResourceView(3, 2, RhiShaderVisibility::All);        // (t2) MaterialTable

    rsDesc->SetAsDescriptorTable(4, 1, RhiShaderVisibility::All);       
    rsDesc->SetDescriptorTableRange(4, RhiDescriptorType::Srv, 1, 0, 3);    // (t3) AccumulationTexture
    rsDesc->SetAsDescriptorTable(5, 1, RhiShaderVisibility::All);      
    rsDesc->SetDescriptorTableRange(5, RhiDescriptorType::Srv, 1, 0, 4);    // (t4) GBuffer0
    rsDesc->SetAsDescriptorTable(6, 1, RhiShaderVisibility::All);      
    rsDesc->SetDescriptorTableRange(6, RhiDescriptorType::Srv, 1, 0, 5);    // (t5) GBuffer1
    rsDesc->SetAsDescriptorTable(7, 1, RhiShaderVisibility::All);
    rsDesc->SetDescriptorTableRange(7, RhiDescriptorType::Srv, 1, 0, 6);    // (t6) GBuffer2
    rsDesc->SetAsDescriptorTable(8, 1, RhiShaderVisibility::All);       
    rsDesc->SetDescriptorTableRange(8, RhiDescriptorType::Uav, 1, 0, 0);    // (u0) LightingTexture
    rsDesc->SetAsDescriptorTable(9, 1, RhiShaderVisibility::All);       
    rsDesc->SetDescriptorTableRange(9, RhiDescriptorType::Uav, 1, 0, 1);    // (u1) IndirectTexture

    rsDesc->SetFlags(RhiRootSignatureFlag::DirectlyIndexed);
    m_GlobalRootSignature = rsDesc->Compile((GetName() + " Root Signature").c_str());
}

void Ether::Graphics::RaytracedLightingProducer::CreatePipelineState(ResourceContext& rc)
{
    m_RTPsoDesc = GraphicCore::GetDevice().CreateRaytracingPipelineStateDesc();
    m_RTPsoDesc->SetLibraryShader(*m_Shader);
    m_RTPsoDesc->SetHitGroupName(k_HitGroupName);
    m_RTPsoDesc->SetClosestHitShaderName(k_ClosestHitShader);
    m_RTPsoDesc->SetMissShaderName(k_MissShader);
    m_RTPsoDesc->SetRayGenShaderName(k_RayGenShader);
    m_RTPsoDesc->SetMaxRecursionDepth(3);
    m_RTPsoDesc->SetMaxAttributeSize(sizeof(float) * 2); // from built in attributes
    m_RTPsoDesc->SetMaxPayloadSize(24);
    m_RTPsoDesc->SetRootSignature(*m_GlobalRootSignature);

    uint32_t numExports = sizeof(s_EntryPoints) / sizeof(s_EntryPoints[0]);

    m_RTPsoDesc->PushLibrary(s_EntryPoints, numExports);
    m_RTPsoDesc->PushHitProgram();
    m_RTPsoDesc->PushShaderConfig();
    m_RTPsoDesc->PushExportAssociation(s_EntryPoints, numExports);
    m_RTPsoDesc->PushPipelineConfig();
    m_RTPsoDesc->PushGlobalRootSignature();

    rc.RegisterPipelineState((GetName() + " Raytracing Pipeline State").c_str(), *m_RTPsoDesc);
}

void Ether::Graphics::RaytracedLightingProducer::InitializeShaderBindingTable(ResourceContext& rc)
{
    const GraphicDisplay& gfxDisplay = GraphicCore::GetGraphicDisplay();

    RhiRaytracingShaderBindingTableDesc desc = {};
    desc.m_MaxRootSignatureSize = 0; // Only ever use global root signature since we are bindless
    desc.m_RaytracingPipelineState = &(RhiRaytracingPipelineState&)rc.GetPipelineState(*m_RTPsoDesc);
    desc.m_HitGroupName = k_HitGroupName;
    desc.m_MissShaderName = k_MissShader;
    desc.m_RayGenShaderName = k_RayGenShader;

    m_RaytracingShaderBindingTable = &rc.CreateRaytracingShaderBindingTable("RT Bindings Table", desc);
}
