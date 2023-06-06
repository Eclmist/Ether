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
#include "graphics/context/commandcontext.h"
#include "graphics/context/graphiccontext.h"

void Ether::Graphics::GraphicCore::Initialize()
{
    m_RhiModule = RhiModule::InitForPlatform();
    m_RhiDevice = m_RhiModule->CreateDevice();

    if (m_Config.GetUseShaderDaemon())
        m_ShaderDaemon = std::make_unique<ShaderDaemon>();

    m_BindlessResourceManager = std::make_unique<BindlessResourceManager>();
    m_RtvAllocator = std::make_unique<DescriptorAllocator>(RhiDescriptorHeapType::Rtv, _4KiB);
    m_DsvAllocator = std::make_unique<DescriptorAllocator>(RhiDescriptorHeapType::Dsv, _4KiB);
    m_SrvCbvUavAllocator = std::make_unique<DescriptorAllocator>(RhiDescriptorHeapType::SrvCbvUav, _64KiB);
    m_CommandManager = std::make_unique<CommandManager>();
    m_GraphicCommon = std::make_unique<GraphicCommon>();
    m_GraphicDisplay = std::make_unique<GraphicDisplay>();
    m_GraphicRenderer = std::make_unique<GraphicRenderer>();

    m_GraphicDisplay->Present();
}

void Ether::Graphics::GraphicCore::Shutdown()
{
    FlushGpu();

    m_GraphicRenderer.reset();
    m_GraphicDisplay.reset();
    m_GraphicCommon.reset();
    m_CommandManager.reset();
    m_SrvCbvUavAllocator.reset();
    m_DsvAllocator.reset();
    m_RtvAllocator.reset();
    m_BindlessResourceManager.reset();
    m_ShaderDaemon.reset();

    m_RhiDevice.reset();
    m_RhiModule.reset();
}

void Ether::Graphics::GraphicCore::FlushGpu()
{
    GetCommandManager().Flush();
}

void Ether::Graphics::GraphicCore::Main()
{
    ETH_MARKER_EVENT("Graphics Update");

    s_Instance->m_GraphicRenderer->WaitForPresent();
    s_Instance->m_GraphicRenderer->Render();
    s_Instance->m_GraphicRenderer->Present();
}
