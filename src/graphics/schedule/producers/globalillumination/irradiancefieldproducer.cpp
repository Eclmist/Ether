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

#include "irradiancefieldproducer.h"

#include "graphics/graphiccore.h"
#include "graphics/rhi/rhiraytracingpipelinestate.h"
#include "graphics/resources/staticmesh.h"
#include "graphics/resources/material.h"
#include "graphics/shaders/common/raytracingconstants.h"
#include "graphics/shaders/common/globalconstants.h"

DEFINE_GFX_PA(IrradianceFieldProducer)
DEFINE_GFX_PA(IrradianceFieldVisualizationProducer)

DECLARE_GFX_SR(RTGeometryInfo)
DECLARE_GFX_AS(RTRaytracingTlas)
DECLARE_GFX_RT(SceneColor)
DECLARE_GFX_DS(SceneDepth)
DECLARE_GFX_CB(GlobalConstants)
DECLARE_GFX_SR(MaterialTable)

DEFINE_GFX_UA_SR(IrradianceFieldIrradianceAtlas)
DEFINE_GFX_UA_SR(IrradianceFieldDepthAtlas)
DEFINE_GFX_CB(IrradianceFieldParams)

static const wchar_t* k_RayGenShader = L"RayGeneration";
static const wchar_t* k_MissShader = L"Miss";
static const wchar_t* k_ClosestHitShader = L"ClosestHit";
static const wchar_t* k_AnyHitShader = L"AnyHit";
static const wchar_t* k_HitGroupName = L"HitGroup";
static const wchar_t* s_EntryPoints[] = { k_RayGenShader, k_MissShader, k_ClosestHitShader, k_AnyHitShader };

Ether::Graphics::IrradianceFieldProducer::IrradianceFieldProducer()
    : GraphicProducer("IrradianceFieldProducer")
{
}

void Ether::Graphics::IrradianceFieldProducer::Initialize(ResourceContext& rc)
{
    CreateShaders();
    CreateRootSignature();
    CreatePipelineState(rc);
}

void Ether::Graphics::IrradianceFieldProducer::GetInputOutput(ScheduleContext& schedule, ResourceContext& rc)
{
    schedule.Read(ACCESS_GFX_SR(RTGeometryInfo));
    schedule.Read(ACCESS_GFX_AS(RTRaytracingTlas));
    schedule.Read(ACCESS_GFX_CB(GlobalConstants));
    schedule.Read(ACCESS_GFX_SR(MaterialTable));

    Shader::IrradianceFieldParams params;
    IrradianceFieldProducer::FillCommonParameters(params);
    schedule.NewCB(ACCESS_GFX_CB(IrradianceFieldParams), AlignUp(sizeof(Shader::IrradianceFieldParams), 256) * GraphicCore::GetGraphicDisplay().GetNumBuffers());
    schedule.NewUA(ACCESS_GFX_UA(IrradianceFieldIrradianceAtlas), params.m_IrradianceAtlasResolution.x, params.m_IrradianceAtlasResolution.y, RhiFormat::R11G11B10Float, RhiResourceDimension::Texture2D);
    schedule.NewSR(ACCESS_GFX_SR(IrradianceFieldIrradianceAtlas), params.m_IrradianceAtlasResolution.x, params.m_IrradianceAtlasResolution.y, RhiFormat::R11G11B10Float, RhiResourceDimension::Texture2D);
    schedule.NewUA(ACCESS_GFX_UA(IrradianceFieldDepthAtlas), params.m_DepthAtlasResolution.x, params.m_DepthAtlasResolution.y, RhiFormat::R16G16Float, RhiResourceDimension::Texture2D);
    schedule.NewSR(ACCESS_GFX_SR(IrradianceFieldDepthAtlas), params.m_DepthAtlasResolution.x, params.m_DepthAtlasResolution.y, RhiFormat::R16G16Float, RhiResourceDimension::Texture2D);

    InitializeShaderBindingTable(rc);
}

void Ether::Graphics::IrradianceFieldProducer::RenderFrame(GraphicContext& ctx, ResourceContext& rc)
{
    ETH_MARKER_EVENT("IrradianceFieldProducer");

    const RhiDevice& gfxDevice = GraphicCore::GetDevice();
    const GraphicDisplay& gfxDisplay = GraphicCore::GetGraphicDisplay();
    const GraphicConfig& config = GraphicCore::GetGraphicConfig();
    const std::vector<Visual>& raytracedVisuals = GraphicCore::GetGraphicRenderer().GetThreadedRenderData().m_RaytracingVisuals;

    ctx.PushMarker("Irradiance Field Probe Trace");
    ctx.SetSrvCbvUavDescriptorHeap(GraphicCore::GetSrvCbvUavAllocator().GetDescriptorHeap());
    ctx.SetSamplerDescriptorHeap(GraphicCore::GetSamplerAllocator().GetDescriptorHeap());
    ctx.SetComputeRootSignature(*m_RootSignature);
    ctx.SetRaytracingShaderBindingTable(m_TraceProbesSBT);
    ctx.SetRaytracingPipelineState((RhiRaytracingPipelineState&)rc.GetPipelineState(*m_TraceProbesPsoDesc));

    ctx.Bind(ACCESS_GFX_CB(GlobalConstants), GetRingBufferOffset());
    ctx.Bind(ACCESS_GFX_SR(MaterialTable));
    ctx.Bind(ACCESS_GFX_AS(RTRaytracingTlas));
    ctx.Bind(ACCESS_GFX_SR(RTGeometryInfo));
    ctx.Bind(ACCESS_GFX_SR(IrradianceFieldIrradianceAtlas));
    ctx.Bind(ACCESS_GFX_UA(IrradianceFieldIrradianceAtlas));
    ctx.Bind(ACCESS_GFX_SR(IrradianceFieldDepthAtlas));
    ctx.Bind(ACCESS_GFX_UA(IrradianceFieldDepthAtlas));

    auto alloc = GetFrameAllocator().Allocate({ sizeof(Shader::IrradianceFieldParams), 256 });
    Shader::IrradianceFieldParams* params = (Shader::IrradianceFieldParams*)alloc->GetCpuHandle();
    IrradianceFieldProducer::FillCommonParameters(*params);

    ctx.CopyBufferRegion(
        dynamic_cast<UploadBufferAllocation&>(*alloc).GetResource(),
        *rc.GetResource(ACCESS_GFX_CB(IrradianceFieldParams)),
        alloc->GetSize(),
        0,
        alloc->GetSize() * GraphicCore::GetGraphicDisplay().GetBackBufferIndex());
    ctx.Bind(ACCESS_GFX_CB(IrradianceFieldParams), alloc->GetSize() * GraphicCore::GetGraphicDisplay().GetBackBufferIndex());

    // For now, trace every probe for simplicity. A dynamically allocated ray budget is possible here, and necessary for
    // lower LODs Depth and irradiance will be traced at the same time
    ctx.DispatchRays(params->m_GridResolution.x, params->m_GridResolution.y, params->m_GridResolution.z);
    ctx.PopMarker();
}

bool Ether::Graphics::IrradianceFieldProducer::IsEnabled()
{
    if (!GraphicCore::GetGraphicConfig().m_IsRaytracingEnabled)
        return false;

    // if (GraphicCore::GetGraphicConfig().m_LightingMode != RaytracingMode::ReSTIR)
    //     return false;

    // if (GraphicCore::GetGraphicRenderer().GetThreadedRenderData().m_RaytracingVisuals.empty())
    //     return false;

    // TODO: Add "Global Illumination mode"

    return true;
}

void Ether::Graphics::IrradianceFieldProducer::CreateShaders()
{
    const RhiDevice& gfxDevice = GraphicCore::GetDevice();
    m_TraceProbesShader = gfxDevice.CreateShader({ "lighting\\globalillumination\\irradiancefieldprobetracing_rgs.hlsl", "", RhiShaderType::Library });
    m_TraceProbesShader->Compile();
    GraphicCore::GetShaderDaemon().RegisterShader(*m_TraceProbesShader);
}

void Ether::Graphics::IrradianceFieldProducer::CreateRootSignature()
{
    const std::vector<const RhiShaderReflection*>& shaderReflections = { &m_TraceProbesShader->GetReflection() };
    m_RootSignature = GraphicCore::GetDevice().CreateRootSignatureDesc(shaderReflections)->Compile((GetName() + " Root Signature").c_str());
}

void Ether::Graphics::IrradianceFieldProducer::CreatePipelineState(ResourceContext& rc)
{
    uint32_t numExports = sizeof(s_EntryPoints) / sizeof(s_EntryPoints[0]);
    m_TraceProbesPsoDesc = GraphicCore::GetDevice().CreateRaytracingPipelineStateDesc();
    m_TraceProbesPsoDesc->SetLibraryShader(*m_TraceProbesShader);
    m_TraceProbesPsoDesc->SetHitGroupName(k_HitGroupName);
    m_TraceProbesPsoDesc->SetClosestHitShaderName(k_ClosestHitShader);
    m_TraceProbesPsoDesc->SetMissShaderName(k_MissShader);
    m_TraceProbesPsoDesc->SetAnyHitShaderName(k_AnyHitShader);
    m_TraceProbesPsoDesc->SetRayGenShaderName(k_RayGenShader);
    m_TraceProbesPsoDesc->SetMaxRecursionDepth(2);
    m_TraceProbesPsoDesc->SetMaxAttributeSize(sizeof(float) * 2); // from built in attributes
    m_TraceProbesPsoDesc->SetMaxPayloadSize(sizeof(Shader::RayPayload) + 4);
    m_TraceProbesPsoDesc->SetRootSignature(*m_RootSignature);
    m_TraceProbesPsoDesc->PushLibrary(s_EntryPoints, numExports);
    m_TraceProbesPsoDesc->PushHitProgram();
    m_TraceProbesPsoDesc->PushShaderConfig();
    m_TraceProbesPsoDesc->PushExportAssociation(s_EntryPoints, numExports);
    m_TraceProbesPsoDesc->PushPipelineConfig();
    m_TraceProbesPsoDesc->PushGlobalRootSignature();
    rc.RegisterPipelineState((GetName() + "Irradiance Field Probe Tracing Pipeline State").c_str(), *m_TraceProbesPsoDesc);
}

void Ether::Graphics::IrradianceFieldProducer::InitializeShaderBindingTable(ResourceContext& rc)
{
    const GraphicDisplay& gfxDisplay = GraphicCore::GetGraphicDisplay();

    RhiRaytracingShaderBindingTableDesc desc = {};
    desc.m_MaxRootSignatureSize = 0; // Only ever use global root signature since we are bindless
    desc.m_RaytracingPipelineState = &(RhiRaytracingPipelineState&)rc.GetPipelineState(*m_TraceProbesPsoDesc);
    desc.m_HitGroupName = k_HitGroupName;
    desc.m_MissShaderName = k_MissShader;
    desc.m_RayGenShaderName = k_RayGenShader;
    m_TraceProbesSBT = &rc.CreateRaytracingShaderBindingTable("Irradiance Probe Trace Bindings Table", desc);
}

void Ether::Graphics::IrradianceFieldProducer::FillCommonParameters(Shader::IrradianceFieldParams& params)
{
    const GraphicConfig& config = GraphicCore::GetGraphicConfig();
    params.m_GridSpacing = config.m_IrradianceFieldGridSpacing;
    params.m_GridOrigin = config.m_IrradianceFieldGridOrigin;
    params.m_GridResolution = config.m_IrradianceFieldGridResolution;
    params.m_IrradianceTileSize = config.m_IrradianceTileSize + 2; // 1px of border on each edge
    params.m_DepthTileSize = config.m_DepthTileSize;
    params.m_IrradianceAtlasResolution.x = params.m_GridResolution.x * params.m_GridResolution.y * params.m_IrradianceTileSize;
    params.m_IrradianceAtlasResolution.y = params.m_GridResolution.z * params.m_IrradianceTileSize;
    params.m_DepthAtlasResolution.x = params.m_GridResolution.x * params.m_GridResolution.y * params.m_DepthTileSize;
    params.m_DepthAtlasResolution.y = params.m_GridResolution.z * params.m_DepthTileSize;

    params.m_VisualizeProbeRadius = config.m_IrradianceFieldVisualizeProbeRadius;
}

Ether::Graphics::IrradianceFieldVisualizationProducer::IrradianceFieldVisualizationProducer()
    : GraphicProducer("IrradianceFieldProbeVisualize")
{
}

void Ether::Graphics::IrradianceFieldVisualizationProducer::Initialize(ResourceContext& rc)
{
    CreateShaders();
    CreateRootSignature();
    CreatePipelineState(rc);
}

void Ether::Graphics::IrradianceFieldVisualizationProducer::GetInputOutput(
    ScheduleContext& schedule,
    ResourceContext& rc)
{
    schedule.Read(ACCESS_GFX_CB(GlobalConstants));
    schedule.Read(ACCESS_GFX_SR(IrradianceFieldIrradianceAtlas));
    schedule.Read(ACCESS_GFX_SR(IrradianceFieldDepthAtlas));
    schedule.Read(ACCESS_GFX_RT(SceneColor));
    schedule.Read(ACCESS_GFX_DS(SceneDepth));
}

void Ether::Graphics::IrradianceFieldVisualizationProducer::RenderFrame(GraphicContext& ctx, ResourceContext& rc)
{
    ETH_MARKER_EVENT("Visualize Irradiance Probes");

    const RhiDevice& gfxDevice = GraphicCore::GetDevice();
    const GraphicDisplay& gfxDisplay = GraphicCore::GetGraphicDisplay();
    const GraphicConfig& config = GraphicCore::GetGraphicConfig();

    ctx.TransitionResource(gfxDisplay.GetBackBuffer(), RhiResourceState::RenderTarget);
    ctx.TransitionResource(*rc.GetResource(ACCESS_GFX_DS(SceneDepth)), RhiResourceState::DepthWrite);

    ctx.SetViewport(gfxDisplay.GetViewport());
    ctx.SetScissorRect(gfxDisplay.GetScissorRect());
    ctx.SetPrimitiveTopology(RhiPrimitiveTopology::TriangleList);
    ctx.SetSrvCbvUavDescriptorHeap(GraphicCore::GetSrvCbvUavAllocator().GetDescriptorHeap());
    ctx.SetSamplerDescriptorHeap(GraphicCore::GetSamplerAllocator().GetDescriptorHeap());
    ctx.SetGraphicRootSignature(*m_RootSignature);
    ctx.SetGraphicPipelineState((RhiGraphicPipelineState&)rc.GetPipelineState(*m_PsoDesc));
    ctx.SetRenderTarget(*ACCESS_GFX_RT(SceneColor), &(*ACCESS_GFX_DS(SceneDepth)));

    Shader::IrradianceFieldParams params;
    IrradianceFieldProducer::FillCommonParameters(params);
    ctx.Bind(ACCESS_GFX_CB(IrradianceFieldParams), AlignUp(sizeof(Shader::IrradianceFieldParams), 256) * GraphicCore::GetGraphicDisplay().GetBackBufferIndex());
    ctx.Bind(ACCESS_GFX_CB(GlobalConstants), GetRingBufferOffset());
    ctx.Bind(ACCESS_GFX_SR(IrradianceFieldIrradianceAtlas));
    ctx.Bind(ACCESS_GFX_SR(IrradianceFieldDepthAtlas));

    const uint32_t numProbes = params.m_GridResolution.x * params.m_GridResolution.y * params.m_GridResolution.z;
    ctx.DrawInstanced(6, numProbes);
}

bool Ether::Graphics::IrradianceFieldVisualizationProducer::IsEnabled()
{
    if (!GraphicCore::GetGraphicConfig().m_IsRaytracingEnabled)
        return false;

    // TODO: Add toggle

    return true;
}

void Ether::Graphics::IrradianceFieldVisualizationProducer::CreateShaders()
{
    const RhiDevice& gfxDevice = GraphicCore::GetDevice();
    m_VertexShader = gfxDevice.CreateShader({ "lighting\\globalillumination\\irradiancefieldvisualize_vsps.hlsl", "VS_Main", RhiShaderType::Vertex });
    m_PixelShader = gfxDevice.CreateShader({ "lighting\\globalillumination\\irradiancefieldvisualize_vsps.hlsl", "PS_Main", RhiShaderType::Pixel });
    m_VertexShader->Compile();
    m_PixelShader->Compile();
    GraphicCore::GetShaderDaemon().RegisterShader(*m_VertexShader);
    GraphicCore::GetShaderDaemon().RegisterShader(*m_PixelShader);
}

void Ether::Graphics::IrradianceFieldVisualizationProducer::CreateRootSignature()
{
    const std::vector<const RhiShaderReflection*>& shaderReflections = { &m_VertexShader->GetReflection(),
                                                                         &m_PixelShader->GetReflection() };
    m_RootSignature = GraphicCore::GetDevice().CreateRootSignatureDesc(shaderReflections)->Compile("Probe Visualize Root Signature");
}

void Ether::Graphics::IrradianceFieldVisualizationProducer::CreatePipelineState(ResourceContext& rc)
{
    m_PsoDesc = GraphicCore::GetDevice().CreateGraphicPipelineStateDesc();
    m_PsoDesc->SetVertexShader(*m_VertexShader);
    m_PsoDesc->SetPixelShader(*m_PixelShader);
    m_PsoDesc->SetRenderTargetFormat(BackBufferHdrFormat);
    m_PsoDesc->SetRootSignature(*m_RootSignature);
    m_PsoDesc->SetInputLayout(nullptr, 0);
    m_PsoDesc->SetDepthTargetFormat(DepthBufferDsvFormat);
    m_PsoDesc->SetDepthStencilState(GraphicCore::GetGraphicCommon().m_DepthStateReadWrite);
    m_PsoDesc->SetBlendState(GraphicCore::GetGraphicCommon().m_BlendDisabled);
    rc.RegisterPipelineState((GetName() + " Pipeline State").c_str(), *m_PsoDesc);
}
