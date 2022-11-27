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
#include "graphics/context/commandcontext.h"
#include "graphics/context/graphiccontext.h"

Ether::Graphics::Core::~Core()
{
    FlushGpu();
    m_Imgui.reset();
}

void Ether::Graphics::Core::Initialize()
{
    m_RhiModule = RhiModule::InitForPlatform();
    m_RhiDevice = m_RhiModule->CreateDevice();

    m_ShaderDaemon = std::make_unique<ShaderDaemon>();
    m_BindlessResourceManager = std::make_unique<BindlessResourceManager>();
    m_GpuDescriptorAllocator = std::make_unique<DescriptorAllocator>(RhiDescriptorHeapType::CbvSrvUav, true);
    m_UploadBufferAllocator = std::make_unique<UploadBufferAllocator>();
    m_CommandManager = std::make_unique<CommandManager>();
    m_GraphicsCommon = std::make_unique<GraphicCommon>();
    m_GraphicsDisplay = std::make_unique<GraphicDisplay>();

    m_Imgui = RhiImguiWrapper::InitForPlatform();
}

void Ether::Graphics::Core::FlushGpu()
{
    GetCommandManager().Flush();
}

#include "graphics/rhi/rhishader.h"

void Ether::Graphics::Core::MainGraphicsThread()
{
    ETH_MARKER_EVENT("Graphics Update");

    RhiShaderDesc shaderDesc = {};
    shaderDesc.m_Filename = "default.hlsl";
    shaderDesc.m_EntryPoint = "VS_Main";
    shaderDesc.m_Type = RhiShaderType::Vertex;
    static std::unique_ptr<RhiShader> tempVs = GetDevice().CreateShader(shaderDesc);

    shaderDesc.m_Filename = "default.hlsl";
    shaderDesc.m_EntryPoint = "PS_Main";
    shaderDesc.m_Type = RhiShaderType::Pixel;
    static std::unique_ptr<RhiShader> tempPs = GetDevice().CreateShader(shaderDesc);

    static std::unique_ptr<RhiPipelineStateDesc> psoDesc = s_Instance->m_RhiDevice->CreatePipelineStateDesc();
    psoDesc->SetVertexShader(*tempVs);
    psoDesc->SetPixelShader(*tempPs);
    psoDesc->SetRenderTargetFormat(BackBufferFormat);
    psoDesc->SetRootSignature(*s_Instance->m_GraphicsCommon->m_EmptyRootSignature);

    GraphicContext tempContext;
    tempContext.PushMarker("Clear");
    tempContext.SetViewport(s_Instance->m_GraphicsDisplay->GetViewport());
    tempContext.SetScissorRect(s_Instance->m_GraphicsDisplay->GetScissorRect());
    tempContext.TransitionResource(s_Instance->m_GraphicsDisplay->GetCurrentBackBuffer(), RhiResourceState::RenderTarget);
    tempContext.ClearColor(s_Instance->m_GraphicsDisplay->GetCurrentBackBufferRtv(), { (float)sin(Time::GetCurrentTime() / 100.0f) / 2.0f + 0.5f, 1, 1, 1 });
    tempContext.PopMarker();
    tempContext.FinalizeAndExecute();
    tempContext.Reset();

    tempContext.PushMarker("Fullscreen");
    tempContext.SetViewport(s_Instance->m_GraphicsDisplay->GetViewport());
    tempContext.SetScissorRect(s_Instance->m_GraphicsDisplay->GetScissorRect());
    tempContext.SetPipelineState(*psoDesc);
    tempContext.SetDescriptorHeap(s_Instance->m_GpuDescriptorAllocator->GetDescriptorHeap());
    tempContext.SetRootSignature(*s_Instance->m_GraphicsCommon->m_EmptyRootSignature);
    tempContext.SetPrimitiveTopology(RhiPrimitiveTopology::TriangleStrip);
    tempContext.SetRenderTarget(s_Instance->m_GraphicsDisplay->GetCurrentBackBufferRtv());
    tempContext.DrawInstanced(4, 1);

    tempContext.TransitionResource(s_Instance->m_GraphicsDisplay->GetCurrentBackBuffer(), RhiResourceState::Present);
    tempContext.PopMarker();
    tempContext.FinalizeAndExecute();
    tempContext.Reset();

    s_Instance->m_Imgui->Render();
    s_Instance->m_GraphicsDisplay->Present();

    s_Instance->m_UploadBufferAllocator->Reset();
    s_Instance->m_FrameNumber++;
}

