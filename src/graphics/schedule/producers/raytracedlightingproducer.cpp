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
#include "graphics/shaders/common/raypayload.h"
#include "graphics/shaders/common/globalconstants.h"

DEFINE_GFX_PA(RaytracedLightingProducer)
DEFINE_GFX_UA(RTLightingTexture)
DEFINE_GFX_SR(RTLightingTexture)
DEFINE_GFX_AS(RTTopLevelAccelerationStructure)

DECLARE_GFX_SR(GBufferTexture0)
DECLARE_GFX_SR(GBufferTexture1)
DECLARE_GFX_SR(GBufferTexture2)
DECLARE_GFX_CB(GlobalRingBuffer)

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
    InitializeShaders();
    InitializeRootSignatures();
    InitializePipelineStates();
}

void Ether::Graphics::RaytracedLightingProducer::GetInputOutput(ScheduleContext& schedule, ResourceContext& rc)
{
    ethVector2u resolution = GraphicCore::GetGraphicConfig().GetResolution();
    schedule.NewUA(ACCESS_GFX_UA(RTLightingTexture), resolution.x, resolution.y, RhiFormat::R32G32B32A32Float, RhiResourceDimension::Texture2D);
    schedule.NewSR(ACCESS_GFX_SR(RTLightingTexture), resolution.x, resolution.y, RhiFormat::R32G32B32A32Float, RhiResourceDimension::Texture2D);
    schedule.NewAS(ACCESS_GFX_AS(RTTopLevelAccelerationStructure), GraphicCore::GetGraphicRenderer().GetRenderData().m_VisualBatch);

    schedule.Read(ACCESS_GFX_SR(GBufferTexture0));
    schedule.Read(ACCESS_GFX_SR(GBufferTexture1));
    schedule.Read(ACCESS_GFX_SR(GBufferTexture2));
    schedule.Read(ACCESS_GFX_CB(GlobalRingBuffer));

    InitializeShaderBindingTable(rc);
}

void Ether::Graphics::RaytracedLightingProducer::RenderFrame(GraphicContext& ctx, ResourceContext& rc)
{
    const RhiDevice& gfxDevice = GraphicCore::GetDevice();
    const GraphicDisplay& gfxDisplay = GraphicCore::GetGraphicDisplay();
    const GraphicConfig& config = GraphicCore::GetGraphicConfig();
    const VisualBatch& visualBatch = GraphicCore::GetGraphicRenderer().GetRenderData().m_VisualBatch;

    const auto resolution = GraphicCore::GetGraphicConfig().GetResolution();
    ctx.PushMarker("Raytrace");
    ctx.TransitionResource(*rc.GetResource(ACCESS_GFX_UA(RTLightingTexture)), RhiResourceState::UnorderedAccess);
    ctx.SetSrvCbvUavDescriptorHeap(GraphicCore::GetSrvCbvUavAllocator().GetDescriptorHeap());
    ctx.SetSamplerDescriptorHeap(GraphicCore::GetSamplerAllocator().GetDescriptorHeap());
    ctx.SetComputeRootSignature(*m_GlobalRootSignature);

    uint64_t ringBufferOffset = gfxDisplay.GetBackBufferIndex() * sizeof(Shader::GlobalConstants);
    ctx.SetComputeRootConstantBufferView(0, rc.GetResource(ACCESS_GFX_CB(GlobalRingBuffer))->GetGpuAddress() + ringBufferOffset);
    ctx.SetComputeRootShaderResourceView(1, rc.GetResource(ACCESS_GFX_AS(RTTopLevelAccelerationStructure))->GetGpuAddress());
    ctx.SetComputeRootDescriptorTable(2, ACCESS_GFX_SR(GBufferTexture0)->GetGpuAddress());
    ctx.SetComputeRootDescriptorTable(3, ACCESS_GFX_SR(GBufferTexture1)->GetGpuAddress());
    ctx.SetComputeRootDescriptorTable(4, ACCESS_GFX_SR(GBufferTexture2)->GetGpuAddress());
    ctx.SetComputeRootDescriptorTable(5, ACCESS_GFX_UA(RTLightingTexture)->GetGpuAddress());
    ctx.SetRaytracingShaderBindingTable(m_RaytracingShaderBindingTable);
    ctx.SetRaytracingPipelineState(*m_RaytracingPipelineState);
    ctx.DispatchRays(resolution.x, resolution.y, 1);
    ctx.PopMarker();
}

bool Ether::Graphics::RaytracedLightingProducer::IsEnabled()
{
    if (!GraphicCore::GetGraphicConfig().m_IsRaytracingEnabled)
        return false;

    if (GraphicCore::GetGraphicRenderer().GetRenderData().m_VisualBatch.m_Visuals.empty())
        return false;

    return true;
}

void Ether::Graphics::RaytracedLightingProducer::InitializeShaders()
{
    const RhiDevice& gfxDevice = GraphicCore::GetDevice();
    m_Shader = gfxDevice.CreateShader({ "lighting\\raytracedlights.hlsl", "", RhiShaderType::Library });
    // Manually compile shader since raytracing PSO caching has not been implemented yet
    m_Shader->Compile();

    GraphicCore::GetShaderDaemon().RegisterShader(*m_Shader);
}

void Ether::Graphics::RaytracedLightingProducer::InitializeRootSignatures()
{
    std::unique_ptr<RhiRootSignatureDesc> rsDesc = GraphicCore::GetDevice().CreateRootSignatureDesc(6, 0);
    rsDesc->SetAsConstantBufferView(0, 0, RhiShaderVisibility::All);        // (b0) Global Constants    
    rsDesc->SetAsShaderResourceView(1, 0, RhiShaderVisibility::All);        // (t0) TLAS

    rsDesc->SetAsDescriptorTable(2, 1, RhiShaderVisibility::All);       
    rsDesc->SetDescriptorTableRange(2, RhiDescriptorType::Srv, 1, 0, 1);    // (t1) GBuffer0
    rsDesc->SetAsDescriptorTable(3, 1, RhiShaderVisibility::All);       
    rsDesc->SetDescriptorTableRange(3, RhiDescriptorType::Srv, 1, 0, 2);    // (t2) GBuffer1
    rsDesc->SetAsDescriptorTable(4, 1, RhiShaderVisibility::All);       
    rsDesc->SetDescriptorTableRange(4, RhiDescriptorType::Srv, 1, 0, 3);    // (t3) GBuffer2
    rsDesc->SetAsDescriptorTable(5, 1, RhiShaderVisibility::All);       
    rsDesc->SetDescriptorTableRange(5, RhiDescriptorType::Uav, 1, 0, 0);    // (u0) LightingTexture

    rsDesc->SetFlags(RhiRootSignatureFlag::DirectlyIndexed);
    m_GlobalRootSignature = rsDesc->Compile((GetName() + "Root Signature").c_str());
}

void Ether::Graphics::RaytracedLightingProducer::InitializePipelineStates()
{
    const RhiDevice& gfxDevice = GraphicCore::GetDevice();

    RhiRaytracingPipelineStateDesc desc = {};
    desc.m_RayGenShaderName = k_RayGenShader;
    desc.m_MissShaderName = k_MissShader;
    desc.m_ClosestHitShaderName = k_ClosestHitShader;
    desc.m_HitGroupName = k_HitGroupName;
    desc.m_MaxAttributeSize = sizeof(float) * 2; // From builtin attributes
    desc.m_MaxPayloadSize = sizeof(Shader::RayPayload);
    desc.m_MaxRecursionDepth = 1;
    desc.m_LibraryShaderDesc = { m_Shader.get(), s_EntryPoints, sizeof(s_EntryPoints) / sizeof(s_EntryPoints[0]) };
    desc.m_GlobalRootSignature = m_GlobalRootSignature.get();

    m_RaytracingPipelineState = gfxDevice.CreateRaytracingPipelineState(desc);
}

void Ether::Graphics::RaytracedLightingProducer::InitializeShaderBindingTable(ResourceContext& rc)
{
    const GraphicDisplay& gfxDisplay = GraphicCore::GetGraphicDisplay();

    RhiRaytracingShaderBindingTableDesc desc = {};
    desc.m_MaxRootSignatureSize = 0; // Only ever use global root signature since we are bindless
    desc.m_RaytracingPipelineState = m_RaytracingPipelineState.get();
    desc.m_HitGroupName = k_HitGroupName;
    desc.m_MissShaderName = k_MissShader;
    desc.m_RayGenShaderName = k_RayGenShader;

    m_RaytracingShaderBindingTable = &rc.CreateRaytracingShaderBindingTable("RT Bindings Table", desc);
}
