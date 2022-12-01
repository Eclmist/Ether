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
}

void Ether::Graphics::Core::Initialize()
{
    m_RhiModule = RhiModule::InitForPlatform();
    m_RhiDevice = m_RhiModule->CreateDevice();

    m_ShaderDaemon = std::make_unique<ShaderDaemon>();
    m_BindlessResourceManager = std::make_unique<BindlessResourceManager>();
    m_RtvAllocator = std::make_unique<DescriptorAllocator>(RhiDescriptorHeapType::Rtv, false, _4KiB);
    m_DsvAllocator = std::make_unique<DescriptorAllocator>(RhiDescriptorHeapType::Dsv, false, _4KiB);
    m_SrvCbvUavAllocator = std::make_unique<DescriptorAllocator>(RhiDescriptorHeapType::SrvCbvUav, true, _64KiB);
    m_CommandManager = std::make_unique<CommandManager>();
    m_GraphicCommon = std::make_unique<GraphicCommon>();
    m_GraphicDisplay = std::make_unique<GraphicDisplay>();
    m_GraphicRenderer = std::make_unique<GraphicRenderer>();

    m_Imgui = RhiImguiWrapper::InitForPlatform();
}

void Ether::Graphics::Core::FlushGpu()
{
    GetCommandManager().Flush();
}

void Ether::Graphics::Core::MainGraphicsThread()
{
    ETH_MARKER_EVENT("Graphics Update");

    s_Instance->m_GraphicRenderer->WaitForPresent();
    s_Instance->m_GraphicRenderer->Render();
    s_Instance->m_GraphicRenderer->Present();

    s_Instance->m_FrameNumber++;
}

