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

#include "graphics/core.h"
#include "graphics/graphicrenderer.h"
#include "graphics/resources/material.h"
#include "graphics/rhi/rhishader.h"

#include "graphics/resources/mesh.h"
#include "common/stream/filestream.h"

std::vector<std::unique_ptr<Ether::Graphics::Mesh>> g_Meshes;
std::string sceneRootPath = "D:\\Graphics_Projects\\Atelier\\Workspaces\\Debug";

Ether::Graphics::RhiInputElementDesc inputElementDesc[5] =
{
    { "POSITION", 0, Ether::Graphics::RhiFormat::R32G32B32Float, 0, 0xffffffff, Ether::Graphics::RhiInputClassification::PerVertexData, 0 },
    { "NORMAL", 0, Ether::Graphics::RhiFormat::R32G32B32Float, 0, 0xffffffff, Ether::Graphics::RhiInputClassification::PerVertexData, 0 },
    { "TANGENT", 0, Ether::Graphics::RhiFormat::R32G32B32Float, 0, 0xffffffff, Ether::Graphics::RhiInputClassification::PerVertexData, 0 },
    { "BITANGENT", 0, Ether::Graphics::RhiFormat::R32G32B32Float, 0, 0xffffffff, Ether::Graphics::RhiInputClassification::PerVertexData, 0 },
    { "TEXCOORD", 0, Ether::Graphics::RhiFormat::R32G32Float, 0, 0xffffffff, Ether::Graphics::RhiInputClassification::PerVertexData, 0 },
};

Ether::Graphics::GraphicRenderer::GraphicRenderer()
    : m_Context("GraphicRenderer::GraphicContext")
{
    LogGraphicsInfo("Initializing Graphic Renderer");

    for (int i = 0; i < MaxSwapChainBuffers; ++i)
        m_FrameLocalUploadBuffer[i] = std::make_unique<UploadBufferAllocator>();

#ifdef ETH_ENGINE
    for (int i = 0; i < 2; ++i)
    {
        std::string meshPath = std::format("{}\\mesh{}.ether", sceneRootPath, i);

        //if (!Ether::PathUtils::IsValidPath(meshPath))
        //    break;

        g_Meshes.push_back(std::make_unique<Mesh>());
        IFileStream ifstream(meshPath);
        g_Meshes[i]->Deserialize(ifstream);
        g_Meshes[i]->CreateGpuResources();
    }
#endif
}

void Ether::Graphics::GraphicRenderer::WaitForPresent()
{
    ETH_MARKER_EVENT("Renderer - WaitForPresent");
    GraphicDisplay& gfxDisplay = Core::GetGraphicDisplay();
    m_Context.GetCommandQueue().StallForFence(gfxDisplay.GetBackBufferFence());
    m_FrameLocalUploadBuffer[gfxDisplay.GetBackBufferIndex()]->Reset();
}

void Ether::Graphics::GraphicRenderer::Render()
{
    ETH_MARKER_EVENT("Renderer - Render");

    RhiDevice& gfxDevice = Core::GetDevice();
    GraphicDisplay& gfxDisplay = Core::GetGraphicDisplay();
    
    static std::unique_ptr<RhiShader> tempVs = gfxDevice.CreateShader({ "default.hlsl", "VS_Main", RhiShaderType::Vertex });
    static std::unique_ptr<RhiShader> tempPs = gfxDevice.CreateShader({ "default.hlsl", "PS_Main", RhiShaderType::Pixel });
    static std::unique_ptr<RhiRootSignatureDesc> rsDesc = Core::GetDevice().CreateRootSignatureDesc(1, 0);
    rsDesc->SetAsConstantBufferView(0, 0, RhiShaderVisibility::All);
    rsDesc->SetFlags(RhiRootSignatureFlag::AllowIAInputLayout);
    static std::unique_ptr<RhiRootSignature> rootSignature = rsDesc->Compile();

    static std::unique_ptr<RhiPipelineStateDesc> psoDesc = Core::GetDevice().CreatePipelineStateDesc();
    psoDesc->SetVertexShader(*tempVs);
    psoDesc->SetPixelShader(*tempPs);
    psoDesc->SetRenderTargetFormat(BackBufferFormat);
    psoDesc->SetRootSignature(*rootSignature);
    psoDesc->SetInputLayout({ inputElementDesc, sizeof(inputElementDesc) / sizeof(inputElementDesc[0]) });
    //psoDesc->SetDepthTargetFormat(RhiFormat::D24UnormS8Uint);
    psoDesc->SetDepthStencilState(Core::GetGraphicCommon().m_DepthStateDisabled);

    m_Context.PushMarker("Clear");
    m_Context.SetViewport(gfxDisplay.GetViewport());
    m_Context.SetScissorRect(gfxDisplay.GetScissorRect());
    m_Context.TransitionResource(gfxDisplay.GetBackBuffer(), RhiResourceState::RenderTarget);
    m_Context.ClearColor(gfxDisplay.GetBackBufferRtv(), { (float)sin(Time::GetCurrentTime() / 100.0f) / 2.0f + 0.5f, 1, 1, 1 });
    m_Context.PopMarker();
    m_Context.FinalizeAndExecute();
    m_Context.Reset();

    m_Context.PushMarker("Geometry");
    m_Context.SetViewport(gfxDisplay.GetViewport());
    m_Context.SetScissorRect(gfxDisplay.GetScissorRect());
    m_Context.SetPrimitiveTopology(RhiPrimitiveTopology::TriangleList);
    m_Context.SetDescriptorHeap(Core::GetGpuDescriptorAllocator().GetDescriptorHeap());
    m_Context.SetGraphicRootSignature(*rootSignature);
    m_Context.SetPipelineState(*psoDesc);
    m_Context.SetRenderTarget(gfxDisplay.GetBackBufferRtv());

    for (int i = 0; i < g_Meshes.size(); ++i)
    {
        Mesh& mesh = *g_Meshes[i];

        Material material;
        material.SetBaseColor(ethVector4(1, 0, 1, 1));
        material.SetSpecularColor(ethVector4(1, 1, 0, 1));
        material.SetMetalness(0.5f);
        material.SetRoughness(sin(Time::GetCurrentTime() / 100.0f));

        auto alloc = m_FrameLocalUploadBuffer[gfxDisplay.GetBackBufferIndex()]->Allocate({ sizeof(Material), 256 });
        memcpy(alloc->GetCpuHandle(), &material, sizeof(Material));

        m_Context.SetRootConstantBuffer(0, dynamic_cast<UploadBufferAllocation&>(*alloc).GetGpuAddress());
        m_Context.SetVertexBuffer(mesh.GetVertexBufferView());
        m_Context.SetIndexBuffer(mesh.GetIndexBufferView());
        m_Context.DrawIndexedInstanced(mesh.GetNumIndices(), 1);
    }

    m_Context.PopMarker();
    m_Context.FinalizeAndExecute();
    m_Context.Reset();

    Core::GetImguiWrapper().Render();
}

void Ether::Graphics::GraphicRenderer::Present()
{
    ETH_MARKER_EVENT("Renderer - Present");
    GraphicDisplay& gfxDisplay = Core::GetGraphicDisplay();

    m_Context.TransitionResource(gfxDisplay.GetBackBuffer(), RhiResourceState::Present);
    m_Context.FinalizeAndExecute();
    m_Context.Reset();

    gfxDisplay.SetCurrentBackBufferFence(m_Context.GetCommandQueue().GetFinalFenceValue());
    gfxDisplay.Present();
}

