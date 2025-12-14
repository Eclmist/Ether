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

#include "directlightingproducer.h"

#include "graphics/graphiccore.h"
#include "graphics/rhi/rhiraytracingpipelinestate.h"
#include "graphics/resources/staticmesh.h"
#include "graphics/resources/material.h"
#include "graphics/shaders/common/raytracingconstants.h"
#include "graphics/shaders/common/globalconstants.h"

DEFINE_GFX_PA(DirectLightingProducer)

DEFINE_GFX_UA_SR(DirectLightingTexture)

DECLARE_GFX_SR(RTGeometryInfo)
DECLARE_GFX_AS(RTRaytracingTlas)
DECLARE_GFX_SR(SceneDepth)
DECLARE_GFX_SR(GBufferTextureA)
DECLARE_GFX_SR(GBufferTextureB)
DECLARE_GFX_SR(GBufferTextureC)
DECLARE_GFX_CB(GlobalConstants)
DECLARE_GFX_SR(MaterialTable)

static const wchar_t* k_RayGenShader = L"RayGeneration";
static const wchar_t* k_MissShader = L"Miss";
static const wchar_t* k_ClosestHitShader = L"ClosestHit";
static const wchar_t* k_AnyHitShader = L"AnyHit";
static const wchar_t* k_HitGroupName = L"HitGroup";
static const wchar_t* s_EntryPoints[] = { k_RayGenShader, k_MissShader, k_ClosestHitShader, k_AnyHitShader };

Ether::Graphics::DirectLightingProducer::DirectLightingProducer()
    : GraphicProducer("DirectLightingProducer")
{
}

void Ether::Graphics::DirectLightingProducer::Initialize(ResourceContext& rc)
{
    CreateShaders();
    CreateRootSignature();
    CreatePipelineState(rc);
}

void Ether::Graphics::DirectLightingProducer::GetInputOutput(ScheduleContext& schedule, ResourceContext& rc)
{
    const ethVector2u resolution = GraphicCore::GetGraphicConfig().GetResolution();

    schedule.NewUA(ACCESS_GFX_UA(DirectLightingTexture), resolution.x, resolution.y, BackBufferHdrFormat, RhiResourceDimension::Texture2D);
    schedule.NewSR(ACCESS_GFX_SR(DirectLightingTexture), resolution.x, resolution.y, BackBufferHdrFormat, RhiResourceDimension::Texture2D);

    schedule.Read(ACCESS_GFX_SR(RTGeometryInfo));
    schedule.Read(ACCESS_GFX_AS(RTRaytracingTlas));
    schedule.Read(ACCESS_GFX_SR(SceneDepth));
    schedule.Read(ACCESS_GFX_SR(GBufferTextureA));
    schedule.Read(ACCESS_GFX_SR(GBufferTextureB));
    schedule.Read(ACCESS_GFX_SR(GBufferTextureC));
    schedule.Read(ACCESS_GFX_CB(GlobalConstants));
    schedule.Read(ACCESS_GFX_SR(MaterialTable));

    InitializeShaderBindingTable(rc);
}

void Ether::Graphics::DirectLightingProducer::RenderFrame(GraphicContext& ctx, ResourceContext& rc)
{
    ETH_MARKER_EVENT("DirectLightingProducer");

    const RhiDevice& gfxDevice = GraphicCore::GetDevice();
    const GraphicDisplay& gfxDisplay = GraphicCore::GetGraphicDisplay();
    const GraphicConfig& config = GraphicCore::GetGraphicConfig();
    const std::vector<Visual>& visuals = GraphicCore::GetGraphicRenderer().GetThreadedRenderData().m_Visuals;
    const std::vector<Visual>& raytracedVisuals = GraphicCore::GetGraphicRenderer().GetThreadedRenderData().m_RaytracingVisuals;
    const auto resolution = GraphicCore::GetGraphicConfig().GetResolution();

    ctx.PushMarker("Direct & Indirect lighting with ReSTIR GI");
    ctx.SetSrvCbvUavDescriptorHeap(GraphicCore::GetSrvCbvUavAllocator().GetDescriptorHeap());
    ctx.SetSamplerDescriptorHeap(GraphicCore::GetSamplerAllocator().GetDescriptorHeap());
    ctx.SetComputeRootSignature(*m_RootSignature);
    ctx.SetRaytracingShaderBindingTable(m_DirectLightingSBT);
    ctx.SetRaytracingPipelineState((RhiRaytracingPipelineState&)rc.GetPipelineState(*m_PsoDesc));

    ctx.Bind(ACCESS_GFX_CB(GlobalConstants), GetRingBufferOffset());
    ctx.Bind(ACCESS_GFX_SR(MaterialTable));
    ctx.Bind(ACCESS_GFX_AS(RTRaytracingTlas));
    ctx.Bind(ACCESS_GFX_SR(RTGeometryInfo));
    ctx.Bind(ACCESS_GFX_SR(SceneDepth));
    ctx.Bind(ACCESS_GFX_SR(GBufferTextureA));
    ctx.Bind(ACCESS_GFX_SR(GBufferTextureB));
    ctx.Bind(ACCESS_GFX_SR(GBufferTextureC));
    ctx.Bind(ACCESS_GFX_UA(DirectLightingTexture));

    ctx.DispatchRays(resolution.x, resolution.y, 1);
    ctx.PopMarker();
}

bool Ether::Graphics::DirectLightingProducer::IsEnabled()
{
    if (!GraphicCore::GetGraphicConfig().m_IsRaytracingEnabled)
        return false;

    if (GraphicCore::GetGraphicRenderer().GetThreadedRenderData().m_RaytracingVisuals.empty())
        return false;

    return true;
}

void Ether::Graphics::DirectLightingProducer::CreateShaders()
{
    const RhiDevice& gfxDevice = GraphicCore::GetDevice();
    m_LightingEvaluationShader = gfxDevice.CreateShader({ "lighting\\directlighting_rgs.hlsl", "", RhiShaderType::Library });

    // Manually compile shader since raytracing PSO caching has not been implemented yet
    m_LightingEvaluationShader->Compile();

    GraphicCore::GetShaderDaemon().RegisterShader(*m_LightingEvaluationShader);
}

void Ether::Graphics::DirectLightingProducer::CreateRootSignature()
{
    m_RootSignature = GraphicCore::GetDevice().CreateRootSignatureDesc(m_LightingEvaluationShader->GetReflection())->Compile((GetName() + " Root Signature").c_str());
}

void Ether::Graphics::DirectLightingProducer::CreatePipelineState(ResourceContext& rc)
{
    uint32_t numExports = sizeof(s_EntryPoints) / sizeof(s_EntryPoints[0]);
    m_PsoDesc = GraphicCore::GetDevice().CreateRaytracingPipelineStateDesc();
    m_PsoDesc->SetLibraryShader(*m_LightingEvaluationShader);
    m_PsoDesc->SetHitGroupName(k_HitGroupName);
    m_PsoDesc->SetClosestHitShaderName(k_ClosestHitShader);
    m_PsoDesc->SetMissShaderName(k_MissShader);
    m_PsoDesc->SetAnyHitShaderName(k_AnyHitShader);
    m_PsoDesc->SetRayGenShaderName(k_RayGenShader);
    m_PsoDesc->SetMaxRecursionDepth(2);
    m_PsoDesc->SetMaxAttributeSize(sizeof(float) * 2); // from built in attributes
    m_PsoDesc->SetMaxPayloadSize(sizeof(Shader::RayPayload) + 4);
    m_PsoDesc->SetRootSignature(*m_RootSignature);
    m_PsoDesc->PushLibrary(s_EntryPoints, numExports);
    m_PsoDesc->PushHitProgram();
    m_PsoDesc->PushShaderConfig();
    m_PsoDesc->PushExportAssociation(s_EntryPoints, numExports);
    m_PsoDesc->PushPipelineConfig();
    m_PsoDesc->PushGlobalRootSignature();
    rc.RegisterPipelineState((GetName() + " Direct Lighting Raytracing Pipeline State").c_str(), *m_PsoDesc);
}

void Ether::Graphics::DirectLightingProducer::InitializeShaderBindingTable(ResourceContext& rc)
{
    const GraphicDisplay& gfxDisplay = GraphicCore::GetGraphicDisplay();

    RhiRaytracingShaderBindingTableDesc desc = {};
    desc.m_MaxRootSignatureSize = 0; // Only ever use global root signature since we are bindless
    desc.m_RaytracingPipelineState = &(RhiRaytracingPipelineState&)rc.GetPipelineState(*m_PsoDesc);
    desc.m_HitGroupName = k_HitGroupName;
    desc.m_MissShaderName = k_MissShader;
    desc.m_RayGenShaderName = k_RayGenShader;
    m_DirectLightingSBT = &rc.CreateRaytracingShaderBindingTable("RT Bindings Table (Direct Lighting)", desc);
}
