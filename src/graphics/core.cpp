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

void Ether::Graphics::Core::Initialize(const GraphicConfig& config)
{
    m_Config = config;

	m_RhiModule = RhiModule::InitForPlatform();
	m_RhiDevice = m_RhiModule->CreateDevice();

	m_BindlessResourceManager = std::make_unique<BindlessResourceManager>();
	m_GpuDescriptorAllocator = std::make_unique<DescriptorAllocator>(RhiDescriptorHeapType::CbvSrvUav, true);
	m_CommandManager = std::make_unique<CommandManager>();
	m_GraphicsDisplay = std::make_unique<GraphicDisplay>();

    m_Imgui = RhiImguiWrapper::InitForPlatform();
}

void Ether::Graphics::Core::FlushGpu()
{
    GetCommandManager().Flush();
}

void Ether::Graphics::Core::Update()
{
    ETH_MARKER_FRAME("Graphics Update");
    GraphicContext context("Clear Framebuffer Context");
    context.NewFrame();
    context.SetMarker("Render");
    context.TransitionResource(m_Instance->m_GraphicsDisplay->GetCurrentBackBuffer(), RhiResourceState::RenderTarget);
    context.ClearColor(m_Instance->m_GraphicsDisplay->GetCurrentBackBufferRtv(), { (float)sin(Time::GetCurrentTime() / 1000.0f),1,1,1 });
    context.TransitionResource(m_Instance->m_GraphicsDisplay->GetCurrentBackBuffer(), RhiResourceState::Present);
    context.FinalizeAndExecute();

    m_Instance->m_Imgui->Render();
    m_Instance->m_GraphicsDisplay->Present();
}

