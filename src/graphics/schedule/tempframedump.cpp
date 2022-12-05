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

#include "graphics/graphiccore.h"
#include "graphics/schedule/tempframedump.h"
#include "graphics/resources/material.h"
#include "graphics/config/graphicconfig.h"
#include <format>
#include <algorithm>

Ether::ethMatrix4x4 GetTranslationMatrix(const Ether::ethVector3& translation)
{
    return { 1.0f, 0.0f, 0.0f, translation.x,
             0.0f, 1.0f, 0.0f, translation.y,
             0.0f, 0.0f, 1.0f, translation.z,
             0.0f, 0.0f, 0.0f, 1.0f };
}

Ether::ethMatrix4x4 GetScaleMatrix(const Ether::ethVector3& scale)
{
    return { scale.x, 0.0f, 0.0f, 0.0f,
             0.0f, scale.y, 0.0f, 0.0f,
             0.0f, 0.0f, scale.z, 0.0f,
             0.0f, 0.0f, 0.0f, 1.0f };
}
Ether::ethMatrix4x4 GetRotationMatrixX(float rotX)
{
    return { 1.0f, 0.0f, 0.0f, 0.0f,
             0.0f, cos(rotX), -sin(rotX), 0.0f,
             0.0f, sin(rotX), cos(rotX), 0.0f,
             0.0f, 0.0f, 0.0f, 1.0f };
}

Ether::ethMatrix4x4 GetRotationMatrixY(float rotY)
{
    return { cos(rotY), 0.0f, sin(rotY), 0.0f,
             0.0f, 1.0f, 0.0f, 0.0f,
             -sin(rotY), 0.0f, cos(rotY), 0.0f,
             0.0f, 0.0f, 0.0f, 1.0f };
}

Ether::ethMatrix4x4 GetRotationMatrixZ(float rotZ)
{
    return { cos(rotZ), -sin(rotZ), 0.0f, 0.0f,
             sin(rotZ), cos(rotZ), 0.0f, 0.0f,
             0.0f, 0.0f, 1.0f, 0.0f,
             0.0f, 0.0f, 0.0f, 1.0f };
}

Ether::ethMatrix4x4 GetRotationMatrix(const Ether::ethVector3& eulerRotation)
{
    return GetRotationMatrixZ(eulerRotation.z) * GetRotationMatrixY(eulerRotation.y) * GetRotationMatrixX(eulerRotation.x);
}

Ether::ethMatrix4x4 GetPerspectiveMatrixLH(float fovy, float aspect, float znear, float zfar)
{
    const float range = tan(SMath::DegToRad(fovy / 2)) * znear;
    const float left = -range * aspect;
    const float right = range * aspect;
    const float bottom = -range;
    const float top = range;
    const float Q = zfar / (zfar - znear);

    Ether::ethMatrix4x4 dst(0.0f);
    dst.m_Data2D[0][0] = (2 * znear) / (right - left);
    dst.m_Data2D[1][1] = (2 * znear) / (top - bottom);
    dst.m_Data2D[2][2] = Q;
    dst.m_Data2D[2][3] = -znear * Q;
    dst.m_Data2D[3][2] = 1;
    return dst;
}

constexpr Ether::Graphics::RhiFormat DepthBufferFormat = Ether::Graphics::RhiFormat::D24UnormS8Uint;


void Ether::Graphics::TempFrameDump::Setup(ResourceContext& resourceContext)
{
    for (int i = 0; i < MaxSwapChainBuffers; ++i)
        m_FrameLocalUploadBuffer[i] = std::make_unique<UploadBufferAllocator>(_2MiB);

    static RhiClearValue clearValue = { DepthBufferFormat, { 1.0, 0 } };
    static RhiCommitedResourceDesc desc = {};
    desc.m_HeapType = RhiHeapType::Default;
    desc.m_State = RhiResourceState::DepthWrite;
    desc.m_ClearValue = &clearValue;
    desc.m_ResourceDesc = RhiCreateDepthStencilResourceDesc(DepthBufferFormat, GraphicCore::GetGraphicConfig().GetResolution());
    desc.m_Name = "Depth Buffer";

    m_DepthBuffer = GraphicCore::GetDevice().CreateCommittedResource(desc);

    static auto alloc = GraphicCore::GetDsvAllocator().Allocate();

    static RhiDepthStencilViewDesc dsvDesc = {};
    dsvDesc.m_Format = DepthBufferFormat;
    dsvDesc.m_Resource = m_DepthBuffer.get();
    dsvDesc.m_TargetCpuAddr = ((DescriptorAllocation&)(*alloc)).GetCpuAddress();

    dsv = GraphicCore::GetDevice().CreateDepthStencilView(dsvDesc);
    GraphicCore::GetDsvAllocator().Free(std::move(alloc));

    RhiDevice& gfxDevice = GraphicCore::GetDevice();
    GraphicDisplay& gfxDisplay = GraphicCore::GetGraphicDisplay();

    vs = gfxDevice.CreateShader({ "default.hlsl", "VS_Main", RhiShaderType::Vertex });
    ps = gfxDevice.CreateShader({ "default.hlsl", "PS_Main", RhiShaderType::Pixel });

    if (GraphicCore::GetGraphicConfig().GetUseShaderDaemon())
    {
        GraphicCore::GetShaderDaemon().RegisterShader(*vs);
        GraphicCore::GetShaderDaemon().RegisterShader(*ps);
    }

    std::unique_ptr<RhiRootSignatureDesc> rsDesc = GraphicCore::GetDevice().CreateRootSignatureDesc(2, 0);
    rsDesc->SetAsConstantBufferView(0, 0, RhiShaderVisibility::All);
    rsDesc->SetAsConstantBufferView(1, 1, RhiShaderVisibility::All);
    rsDesc->SetFlags(RhiRootSignatureFlag::AllowIAInputLayout);
    rootSignature = rsDesc->Compile();

    psoDesc = GraphicCore::GetDevice().CreatePipelineStateDesc();
    psoDesc->SetVertexShader(*vs);
    psoDesc->SetPixelShader(*ps);
    psoDesc->SetRenderTargetFormat(BackBufferFormat);
    psoDesc->SetRootSignature(*rootSignature);
    psoDesc->SetInputLayout({ m_InputElementDesc, sizeof(m_InputElementDesc) / sizeof(m_InputElementDesc[0]) });
    psoDesc->SetDepthTargetFormat(DepthBufferFormat);
    psoDesc->SetDepthStencilState(GraphicCore::GetGraphicCommon().m_DepthStateReadWrite);

    resourceContext.AddPipelineState(*psoDesc);
}

void Ether::Graphics::TempFrameDump::Render(GraphicContext& graphicContext, ResourceContext& resourceContext)
{
    ETH_MARKER_EVENT("Temp Frame Dump - Render");
    const RhiDevice& gfxDevice = GraphicCore::GetDevice();
    const GraphicDisplay& gfxDisplay = GraphicCore::GetGraphicDisplay();
    const GraphicConfig& config = GraphicCore::GetGraphicConfig();

    graphicContext.PushMarker("Clear");
    graphicContext.SetViewport(gfxDisplay.GetViewport());
    graphicContext.SetScissorRect(gfxDisplay.GetScissorRect());
    graphicContext.TransitionResource(gfxDisplay.GetBackBuffer(), RhiResourceState::RenderTarget);
    graphicContext.ClearColor(gfxDisplay.GetBackBufferRtv(), config.GetClearColor());
    graphicContext.ClearDepthStencil(*dsv, 1.0);
    graphicContext.PopMarker();
    graphicContext.FinalizeAndExecute();
    graphicContext.Reset();

    graphicContext.PushMarker("Geometry");
    graphicContext.SetViewport(gfxDisplay.GetViewport());
    graphicContext.SetScissorRect(gfxDisplay.GetScissorRect());
    graphicContext.SetPrimitiveTopology(RhiPrimitiveTopology::TriangleList);
    graphicContext.SetDescriptorHeap(GraphicCore::GetSrvCbvUavAllocator().GetDescriptorHeap());
    graphicContext.SetGraphicRootSignature(*rootSignature);
    graphicContext.SetPipelineState(resourceContext.GetPipelineState(*psoDesc));

    graphicContext.SetRenderTarget(gfxDisplay.GetBackBufferRtv(), dsv.get());

    UploadGlobalConstants(graphicContext);

    const VisualBatch& visualBatch = graphicContext.GetVisualBatch();

    for (int i = 0; i < visualBatch.m_Visuals.size(); ++i)
    {
        ETH_MARKER_EVENT("Draw Meshes");
        Visual visual = visualBatch.m_Visuals[i];

        auto alloc = m_FrameLocalUploadBuffer[gfxDisplay.GetBackBufferIndex()]->Allocate({ sizeof(Material), 256 });
        memcpy(alloc->GetCpuHandle(), visual.m_Material, sizeof(Material));

        graphicContext.SetRootConstantBuffer(1, dynamic_cast<UploadBufferAllocation&>(*alloc).GetGpuAddress());
        graphicContext.SetVertexBuffer(visual.m_Mesh->GetVertexBufferView());
        graphicContext.SetIndexBuffer(visual.m_Mesh->GetIndexBufferView());
        graphicContext.DrawIndexedInstanced(visual.m_Mesh->GetNumIndices(), 1);
    }

    graphicContext.PopMarker();
    graphicContext.FinalizeAndExecute();
    graphicContext.Reset();
}

void Ether::Graphics::TempFrameDump::Reset()
{
    GraphicDisplay& gfxDisplay = GraphicCore::GetGraphicDisplay();
    m_FrameLocalUploadBuffer[gfxDisplay.GetBackBufferIndex()]->Reset();
}

void Ether::Graphics::TempFrameDump::UploadGlobalConstants(GraphicContext& context)
{
    // Set up global constants
    GlobalConstants globalConstants;
    globalConstants.m_ViewMatrix = context.GetViewMatrix();
    globalConstants.m_ProjectionMatrix = context.GetProjectionMatrix();
    globalConstants.m_EyeDirection = context.GetEyeDirection().Resize<4>();
    globalConstants.m_EyePosition = context.GetEyePosition().Resize<4>();
    globalConstants.m_Time = ethVector4(Time::GetTimeSinceStartup() / 20, Time::GetTimeSinceStartup(), Time::GetTimeSinceStartup() * 2, Time::GetTimeSinceStartup() * 3);
    globalConstants.m_ScreenResolution = GraphicCore::GetGraphicConfig().GetResolution();

    auto alloc = m_FrameLocalUploadBuffer[GraphicCore::GetGraphicDisplay().GetBackBufferIndex()]->Allocate({ sizeof(GlobalConstants), 256 });
    memcpy(alloc->GetCpuHandle(), &globalConstants, sizeof(GlobalConstants));
    context.SetRootConstantBuffer(0, dynamic_cast<UploadBufferAllocation&>(*alloc).GetGpuAddress());
}
