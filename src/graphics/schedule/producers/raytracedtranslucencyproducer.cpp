/*
    This file is part of Ether, an open-source DirectX 12 renderer.

    Copyright (c) 2020-2030 Samuel Huang - All rights reserved.

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

#include "raytracedtranslucencyproducer.h"

#include "graphics/graphiccore.h"
#include "graphics/rhi/rhiraytracingpipelinestate.h"
#include "graphics/resources/staticmesh.h"
#include "graphics/resources/material.h"
#include "graphics/shaders/common/raytracingconstants.h"
#include "graphics/shaders/common/globalconstants.h"

DEFINE_GFX_PA(RaytracedTranslucencyProducer)

DECLARE_GFX_SR(RTGeometryInfo)
DECLARE_GFX_AS(RTRaytracingTlas)
DECLARE_GFX_UA(SceneColor)
DECLARE_GFX_SR(SceneDepth)
DECLARE_GFX_CB(GlobalConstants)
DECLARE_GFX_SR(MaterialTable)

static const wchar_t* k_RayGenShader = L"RayGeneration";
static const wchar_t* k_MissShader = L"Miss";
static const wchar_t* k_ClosestHitShader = L"ClosestHit";
static const wchar_t* k_AnyHitShader = L"AnyHit";
static const wchar_t* k_HitGroupName = L"HitGroup";
static const wchar_t* s_EntryPoints[] = { k_RayGenShader, k_MissShader, k_ClosestHitShader, k_AnyHitShader };

Ether::Graphics::RaytracedTranslucencyProducer::RaytracedTranslucencyProducer()
    : GraphicProducer("RaytracedTranslucencyProducer")
{
}

void Ether::Graphics::RaytracedTranslucencyProducer::Initialize(ResourceContext& rc)
{
    CreateShaders();
    CreateRootSignature();
    CreatePipelineState(rc);
}

void Ether::Graphics::RaytracedTranslucencyProducer::GetInputOutput(ScheduleContext& schedule, ResourceContext& rc)
{
    ethVector2u resolution = GraphicCore::GetGraphicConfig().GetResolution();
    uint32_t numRTVisuals = GraphicCore::GetGraphicRenderer().GetThreadedRenderData().m_RaytracingVisuals.size();

    schedule.Read(ACCESS_GFX_SR(RTGeometryInfo));
    schedule.Read(ACCESS_GFX_AS(RTRaytracingTlas));
    schedule.Read(ACCESS_GFX_UA(SceneColor));
    schedule.Read(ACCESS_GFX_SR(SceneDepth));
    schedule.Read(ACCESS_GFX_CB(GlobalConstants));
    schedule.Read(ACCESS_GFX_SR(MaterialTable));

    InitializeShaderBindingTable(rc);
}

void Ether::Graphics::RaytracedTranslucencyProducer::RenderFrame(GraphicContext& ctx, ResourceContext& rc)
{
    ETH_MARKER_EVENT("RaytracedTranslucencyProducer");

    const RhiDevice& gfxDevice = GraphicCore::GetDevice();
    const GraphicDisplay& gfxDisplay = GraphicCore::GetGraphicDisplay();
    const GraphicConfig& config = GraphicCore::GetGraphicConfig();
    const std::vector<Visual>& visuals = GraphicCore::GetGraphicRenderer().GetThreadedRenderData().m_Visuals;
    const std::vector<Visual>& raytracedVisuals = GraphicCore::GetGraphicRenderer().GetThreadedRenderData().m_RaytracingVisuals;

    const auto resolution = GraphicCore::GetGraphicConfig().GetResolution();
    ctx.PushMarker("Raytrace translucencies");
    ctx.SetSrvCbvUavDescriptorHeap(GraphicCore::GetSrvCbvUavAllocator().GetDescriptorHeap());
    ctx.SetSamplerDescriptorHeap(GraphicCore::GetSamplerAllocator().GetDescriptorHeap());
    ctx.SetComputeRootSignature(*m_GlobalRootSignature);

    uint64_t ringBufferOffset = gfxDisplay.GetBackBufferIndex() * AlignUp(sizeof(Shader::GlobalConstants), 256);
    ctx.SetComputeRootConstantBufferView(0, rc.GetResource(ACCESS_GFX_CB(GlobalConstants))->GetGpuAddress() + ringBufferOffset);
    ctx.SetComputeRootShaderResourceView(1, rc.GetResource(ACCESS_GFX_SR(MaterialTable))->GetGpuAddress());
    ctx.SetComputeRootShaderResourceView(2, rc.GetResource(ACCESS_GFX_AS(RTRaytracingTlas))->GetGpuAddress());
    ctx.SetComputeRootShaderResourceView(3, rc.GetResource(ACCESS_GFX_SR(RTGeometryInfo))->GetGpuAddress());
    ctx.SetComputeRootDescriptorTable(4, ACCESS_GFX_SR(SceneDepth)->GetGpuAddress());
    ctx.SetComputeRootDescriptorTable(5, ACCESS_GFX_UA(SceneColor)->GetGpuAddress());
    ctx.SetRaytracingShaderBindingTable(m_RaytracingShaderBindingTable);
    ctx.SetRaytracingPipelineState((RhiRaytracingPipelineState&)rc.GetPipelineState(*m_RTPsoDesc));
    ctx.DispatchRays(resolution.x, resolution.y, 1);
    ctx.PopMarker();
}

bool Ether::Graphics::RaytracedTranslucencyProducer::IsEnabled()
{
    if (!GraphicCore::GetGraphicConfig().m_IsRaytracingEnabled)
        return false;

    if (!GraphicCore::GetGraphicConfig().m_RaytracedReflectionsEnabled)
        return false;

    if (GraphicCore::GetGraphicRenderer().GetThreadedRenderData().m_RaytracingVisuals.empty())
        return false;

    return true;
}

void Ether::Graphics::RaytracedTranslucencyProducer::CreateShaders()
{
    const RhiDevice& gfxDevice = GraphicCore::GetDevice();
    m_Shader = gfxDevice.CreateShader({ "translucency\\raytracedtranslucency_rgs.hlsl", "", RhiShaderType::Library });
    // Manually compile shader since raytracing PSO caching has not been implemented yet
    m_Shader->Compile();

    GraphicCore::GetShaderDaemon().RegisterShader(*m_Shader);
}

void Ether::Graphics::RaytracedTranslucencyProducer::CreateRootSignature()
{
    std::unique_ptr<RhiRootSignatureDesc> rsDesc = GraphicCore::GetDevice().CreateRootSignatureDesc(6, 0);
    rsDesc->SetAsConstantBufferView(0, 0, RhiShaderVisibility::All);        // (b0) Global Constants    
    rsDesc->SetAsShaderResourceView(1, 0, RhiShaderVisibility::All);        // (t0) MaterialTable
    rsDesc->SetAsShaderResourceView(2, 1, RhiShaderVisibility::All);        // (t1) TLAS
    rsDesc->SetAsShaderResourceView(3, 2, RhiShaderVisibility::All);        // (t2) RTGeometryInfo

    rsDesc->SetAsDescriptorTable(4, 1, RhiShaderVisibility::All);
    rsDesc->SetDescriptorTableRange(4, RhiDescriptorType::Srv, 1, 0, 3);    // (t3) SceneDepth
    rsDesc->SetAsDescriptorTable(5, 1, RhiShaderVisibility::All);       
    rsDesc->SetDescriptorTableRange(5, RhiDescriptorType::Uav, 1, 0, 0);    // (u0) SceneColor

    rsDesc->SetFlags(RhiRootSignatureFlag::DirectlyIndexed);
    m_GlobalRootSignature = rsDesc->Compile((GetName() + " Root Signature").c_str());
}

void Ether::Graphics::RaytracedTranslucencyProducer::CreatePipelineState(ResourceContext& rc)
{
    m_RTPsoDesc = GraphicCore::GetDevice().CreateRaytracingPipelineStateDesc();
    m_RTPsoDesc->SetLibraryShader(*m_Shader);
    m_RTPsoDesc->SetHitGroupName(k_HitGroupName);
    m_RTPsoDesc->SetClosestHitShaderName(k_ClosestHitShader);
    m_RTPsoDesc->SetMissShaderName(k_MissShader);
    m_RTPsoDesc->SetAnyHitShaderName(k_AnyHitShader);
    m_RTPsoDesc->SetRayGenShaderName(k_RayGenShader);
    m_RTPsoDesc->SetMaxRecursionDepth(2);
    m_RTPsoDesc->SetMaxAttributeSize(sizeof(float) * 2); // from built in attributes
    m_RTPsoDesc->SetMaxPayloadSize(sizeof(Shader::TranslucentRayPayload));
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

void Ether::Graphics::RaytracedTranslucencyProducer::InitializeShaderBindingTable(ResourceContext& rc)
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
