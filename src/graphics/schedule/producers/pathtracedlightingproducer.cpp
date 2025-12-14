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

#include "pathtracedlightingproducer.h"

#include "graphics/graphiccore.h"
#include "graphics/rhi/rhiraytracingpipelinestate.h"
#include "graphics/resources/staticmesh.h"
#include "graphics/resources/material.h"
#include "graphics/shaders/common/raytracingconstants.h"
#include "graphics/shaders/common/globalconstants.h"

DEFINE_GFX_PA(PathtracedLightingProducer)
DEFINE_GFX_SR(DiffuseIndirectAccumulationTexture)

DECLARE_GFX_SR(RTGeometryInfo)
DECLARE_GFX_AS(RTRaytracingTlas)
DECLARE_GFX_UA(DirectLightingTexture)
DECLARE_GFX_UA(DiffuseIndirectLightingTexture)
DECLARE_GFX_SR(GBufferTextureA)
DECLARE_GFX_SR(GBufferTextureB)
DECLARE_GFX_SR(GBufferTextureC)
DECLARE_GFX_SR(SceneDepth)
DECLARE_GFX_CB(GlobalConstants)
DECLARE_GFX_SR(MaterialTable)

static const wchar_t* k_RayGenShader = L"RayGeneration";
static const wchar_t* k_MissShader = L"Miss";
static const wchar_t* k_ClosestHitShader = L"ClosestHit";
static const wchar_t* k_AnyHitShader = L"AnyHit";
static const wchar_t* k_HitGroupName = L"HitGroup";
static const wchar_t* s_EntryPoints[] = { k_RayGenShader, k_MissShader, k_ClosestHitShader, k_AnyHitShader };

Ether::Graphics::PathtracedLightingProducer::PathtracedLightingProducer()
    : GraphicProducer("PathtracedLightingProducer")
{
}

void Ether::Graphics::PathtracedLightingProducer::Initialize(ResourceContext& rc)
{
    CreateShaders();
    CreateRootSignature();
    CreatePipelineState(rc);
}

void Ether::Graphics::PathtracedLightingProducer::GetInputOutput(ScheduleContext& schedule, ResourceContext& rc)
{
    ethVector2u resolution = GraphicCore::GetGraphicConfig().GetResolution();

    schedule.NewSR(ACCESS_GFX_SR(DiffuseIndirectAccumulationTexture), resolution.x, resolution.y, BackBufferHdrFormat, RhiResourceDimension::Texture2D);

    schedule.Read(ACCESS_GFX_SR(RTGeometryInfo));
    schedule.Read(ACCESS_GFX_AS(RTRaytracingTlas));
    schedule.Read(ACCESS_GFX_SR(GBufferTextureA));
    schedule.Read(ACCESS_GFX_SR(GBufferTextureB));
    schedule.Read(ACCESS_GFX_SR(GBufferTextureC));
    schedule.Read(ACCESS_GFX_SR(SceneDepth));
    schedule.Read(ACCESS_GFX_CB(GlobalConstants));
    schedule.Read(ACCESS_GFX_SR(MaterialTable));
    schedule.Read(ACCESS_GFX_UA(DiffuseIndirectLightingTexture));

    InitializeShaderBindingTable(rc);
}

void Ether::Graphics::PathtracedLightingProducer::RenderFrame(GraphicContext& ctx, ResourceContext& rc)
{
    ETH_MARKER_EVENT("PathtracedLightingProducer");

    const RhiDevice& gfxDevice = GraphicCore::GetDevice();
    const GraphicDisplay& gfxDisplay = GraphicCore::GetGraphicDisplay();
    const GraphicConfig& config = GraphicCore::GetGraphicConfig();
    const std::vector<Visual>& visuals = GraphicCore::GetGraphicRenderer().GetThreadedRenderData().m_Visuals;
    const auto resolution = GraphicCore::GetGraphicConfig().GetResolution();

    ctx.PushMarker("Direct & Indirect lighting with pathtracing");
    ctx.TransitionResource(*rc.GetResource(ACCESS_GFX_UA(DirectLightingTexture)), RhiResourceState::UnorderedAccess);
    ctx.SetSrvCbvUavDescriptorHeap(GraphicCore::GetSrvCbvUavAllocator().GetDescriptorHeap());
    ctx.SetSamplerDescriptorHeap(GraphicCore::GetSamplerAllocator().GetDescriptorHeap());
    ctx.SetComputeRootSignature(*m_RootSignature);
    ctx.SetRaytracingShaderBindingTable(m_RaytracingShaderBindingTable);
    ctx.SetRaytracingPipelineState((RhiRaytracingPipelineState&)rc.GetPipelineState(*m_RTPsoDesc));
    ctx.Bind(ACCESS_GFX_CB(GlobalConstants), GetRingBufferOffset());
    ctx.Bind(ACCESS_GFX_SR(MaterialTable));
    ctx.Bind(ACCESS_GFX_AS(RTRaytracingTlas));
    ctx.Bind(ACCESS_GFX_SR(RTGeometryInfo));
    ctx.Bind(ACCESS_GFX_SR(GBufferTextureA));
    ctx.Bind(ACCESS_GFX_SR(GBufferTextureB));
    ctx.Bind(ACCESS_GFX_SR(GBufferTextureC));
    ctx.Bind(ACCESS_GFX_SR(SceneDepth));
    ctx.Bind(ACCESS_GFX_UA(DirectLightingTexture));
    ctx.Bind(ACCESS_GFX_UA(DiffuseIndirectLightingTexture));
    ctx.Bind(ACCESS_GFX_SR(DiffuseIndirectAccumulationTexture));
    ctx.DispatchRays(resolution.x, resolution.y, 1);

    // TODO: Move transition into CopyResource
    ctx.TransitionResource(*rc.GetResource(ACCESS_GFX_UA(DiffuseIndirectLightingTexture)), RhiResourceState::CopySrc);
    ctx.TransitionResource(*rc.GetResource(ACCESS_GFX_SR(DiffuseIndirectAccumulationTexture)), RhiResourceState::CopyDest);
    ctx.CopyResource(*rc.GetResource(ACCESS_GFX_UA(DiffuseIndirectLightingTexture)), *rc.GetResource(ACCESS_GFX_SR(DiffuseIndirectAccumulationTexture)));
    ctx.PopMarker();
}

bool Ether::Graphics::PathtracedLightingProducer::IsEnabled()
{
    if (!GraphicCore::GetGraphicConfig().m_IsRaytracingEnabled)
        return false;

    if (GraphicCore::GetGraphicConfig().m_GlobalIlluminationMode != RaytracingMode::Pathtracer)
        return false;

    if (GraphicCore::GetGraphicRenderer().GetThreadedRenderData().m_RaytracingVisuals.empty())
        return false;

    return true;
}

void Ether::Graphics::PathtracedLightingProducer::CreateShaders()
{
    const RhiDevice& gfxDevice = GraphicCore::GetDevice();
    m_Shader = gfxDevice.CreateShader({ "lighting\\pathtracing_rgs.hlsl", "", RhiShaderType::Library });
    // Manually compile shader since raytracing PSO caching has not been implemented yet
    m_Shader->Compile();

    GraphicCore::GetShaderDaemon().RegisterShader(*m_Shader);
}

void Ether::Graphics::PathtracedLightingProducer::CreateRootSignature()
{
    m_RootSignature = GraphicCore::GetDevice().CreateRootSignatureDesc(m_Shader->GetReflection())->Compile((GetName() + " Root Signature").c_str());
}

void Ether::Graphics::PathtracedLightingProducer::CreatePipelineState(ResourceContext& rc)
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
    m_RTPsoDesc->SetMaxPayloadSize(sizeof(Shader::RayPayload) + 4);
    m_RTPsoDesc->SetRootSignature(*m_RootSignature);

    uint32_t numExports = sizeof(s_EntryPoints) / sizeof(s_EntryPoints[0]);

    m_RTPsoDesc->PushLibrary(s_EntryPoints, numExports);
    m_RTPsoDesc->PushHitProgram();
    m_RTPsoDesc->PushShaderConfig();
    m_RTPsoDesc->PushExportAssociation(s_EntryPoints, numExports);
    m_RTPsoDesc->PushPipelineConfig();
    m_RTPsoDesc->PushGlobalRootSignature();

    rc.RegisterPipelineState((GetName() + " Raytracing Pipeline State").c_str(), *m_RTPsoDesc);
}

void Ether::Graphics::PathtracedLightingProducer::InitializeShaderBindingTable(ResourceContext& rc)
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
