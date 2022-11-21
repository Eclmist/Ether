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
#include "graphics/rhi/rhiimguiwrapper.h"
#include "graphics/imgui/imgui.h"

#if defined (ETH_GRAPHICS_DX12)
#include "graphics/rhi/dx12/dx12imguiwrapper.h"
#elif defined (ETH_GRAPHICS_PS5)
#include "graphics/rhi/ps5/ps5imguiwrapper.h"
#endif

Ether::Graphics::RhiImguiWrapper::RhiImguiWrapper()
    : m_Context("Imgui Context")
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
}

Ether::Graphics::RhiImguiWrapper::~RhiImguiWrapper()
{
    ImGui::DestroyContext();
}

void Ether::Graphics::RhiImguiWrapper::Render()
{
    bool demo = true;
    ImGui::NewFrame();
    ImGui::ShowDemoWindow(&demo);
    ImGui::Render();

    m_Context.NewFrame();
    m_Context.TransitionResource(Core::GetGraphicsDisplay().GetCurrentBackBuffer(), RhiResourceState::RenderTarget);
    m_Context.SetRenderTarget(Core::GetGraphicsDisplay().GetCurrentBackBufferRtv());
    m_Context.SetDescriptorHeap(Core::GetGpuDescriptorAllocator().GetDescriptorHeap());

    RenderDrawData();

    m_Context.TransitionResource(Core::GetGraphicsDisplay().GetCurrentBackBuffer(), RhiResourceState::Present);
    m_Context.FinalizeAndExecute();
}

std::unique_ptr<Ether::Graphics::RhiImguiWrapper> Ether::Graphics::RhiImguiWrapper::InitForPlatform()
{
#if defined(ETH_GRAPHICS_DX12)
    return std::make_unique<Dx12ImguiWrapper>();
#else
    static_assert(false, "Not yet implemented");
#endif
}

