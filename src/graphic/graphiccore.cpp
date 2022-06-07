/*
    This file is part of Ether, an open-source DirectX 12 renderer.

    Copyright (c) 2020-2022 Samuel Huang - All rights reserved.

    Ether is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "graphic/rhi/rhidevice.h"

ETH_NAMESPACE_BEGIN

void GraphicCore::Initialize()
{
    if (Instance().m_IsInitialized)
    {
        LogGraphicsWarning("An attempt was made to initialize an already initialized Renderer");
        return;
    }

    Instance().m_RhiModule = RhiModule::CreateModule(RhiModuleType::D3D12);
    Instance().m_RhiModule->Initialize();
    RhiResult result = Instance().m_RhiModule->CreateDevice(Instance().m_RhiDevice);

    AssertGraphics(result == RhiResult::Success, "Failed to create graphic device");

    Instance().m_RTVDescriptorHeap.SetName(L"GraphicCore::RTVDescriptorHeap");
    Instance().m_DSVDescriptorHeap.SetName(L"GraphicCore::DSVDescriptorHeap");
    Instance().m_SRVDescriptorHeap.SetName(L"GraphicCore::SRVDescriptorHeap");
    Instance().m_SamplerDescriptorHeap.SetName(L"GraphicCore::SamplerDescriptorHeap");

    GetDevice()->CreateDescriptorHeap({ RhiDescriptorHeapType::RTV, RhiDescriptorHeapFlag::None, 512 }, Instance().m_RTVDescriptorHeap);
    GetDevice()->CreateDescriptorHeap({ RhiDescriptorHeapType::DSV, RhiDescriptorHeapFlag::None, 512 }, Instance().m_DSVDescriptorHeap);
    GetDevice()->CreateDescriptorHeap({ RhiDescriptorHeapType::CbvSrvUav, RhiDescriptorHeapFlag::ShaderVisible, 4096 }, Instance().m_SRVDescriptorHeap);
    GetDevice()->CreateDescriptorHeap({ RhiDescriptorHeapType::Sampler, RhiDescriptorHeapFlag::None, 512 }, Instance().m_SamplerDescriptorHeap);

    Instance().m_ShaderDaemon = std::make_unique<ShaderDaemon>();
    Instance().m_CommandManager = std::make_unique<CommandManager>();
    Instance().m_GraphicCommon = std::make_unique<GraphicCommon>();
    Instance().m_GraphicDisplay = std::make_unique<GraphicDisplay>();
    Instance().m_GraphicRenderer = std::make_unique<GraphicRenderer>();
    Instance().m_GuiRenderer = std::make_unique<GuiRenderer>();
    Instance().m_IsInitialized = true;
}

void GraphicCore::Render()
{
    Instance().m_GraphicRenderer->WaitForPresent();
    Instance().m_GraphicRenderer->Render();
    Instance().m_GuiRenderer->Render();
    Instance().m_GraphicRenderer->Present();
    Instance().m_GraphicRenderer->CleanUp();
}

void GraphicCore::Shutdown()
{
    FlushGpu();
    Instance().m_GraphicCommon.reset();
    Instance().m_GuiRenderer.reset();
    Instance().m_GraphicRenderer.reset();
    Instance().m_GraphicDisplay.reset();
    Instance().m_CommandManager.reset();
    Instance().m_ShaderDaemon.reset();

    Instance().m_RhiDevice.Destroy();
    Instance().m_RhiModule.Destroy();

    Instance().m_RTVDescriptorHeap.Destroy();
    Instance().m_DSVDescriptorHeap.Destroy();
    Instance().m_SRVDescriptorHeap.Destroy();
    Instance().m_SamplerDescriptorHeap.Destroy();

    Reset();
}

bool GraphicCore::IsInitialized()
{
    return HasInstance() && Instance().m_IsInitialized;
}

void GraphicCore::FlushGpu()
{
    GetCommandManager().Flush();
}

void GraphicCore::InitializeDebugLayer()
{
}

ETH_NAMESPACE_END

