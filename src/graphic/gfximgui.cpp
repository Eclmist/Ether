/*
    This file is part of Ether, an open-source DirectX 12 renderer.

    Copyright (c) 2020-2021 Samuel Van Allen - All rights reserved.

    Ether is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include "gfximgui.h"
#include "graphic/hal/dx12commandlist.h"
#include "graphic/hal/dx12descriptorheap.h"
#include "graphic/hal/dx12device.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx12.h"

GfxImGui::GfxImGui()
    : m_IsVisible(false)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
}

void GfxImGui::Initialize(GfxContext& context, DX12DescriptorHeap& srvDescriptor)
{
    m_Context = &context;

    ImGui_ImplDX12_Init(
        context.GetDevice()->Get().Get(),
        ETH_NUM_SWAPCHAIN_BUFFERS,
        DXGI_FORMAT_R8G8B8A8_UNORM,
        srvDescriptor.Get().Get(),
        srvDescriptor.Get()->GetCPUDescriptorHandleForHeapStart(),
        srvDescriptor.Get()->GetGPUDescriptorHandleForHeapStart()
    );
}

void GfxImGui::Render(DX12CommandList& commandList) const
{
    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    SetupUI();

    ImGui::Render();
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList.Get().Get());
}

void GfxImGui::Shutdown()
{
    ImGui_ImplWin32_Shutdown();
    ImGui_ImplDX12_Shutdown();
    ImGui::DestroyContext();
}

void GfxImGui::ToggleVisible()
{
    SetVisible(!GetVisible());
}

void GfxImGui::SetVisible(bool isVisible)
{
    m_IsVisible = isVisible;
}

bool GfxImGui::GetVisible() const
{
    return m_IsVisible;
}

void GfxImGui::SetupUI() const
{
    static bool showImGuiDemo = false;
    ethVector4 clearColor = m_Context->GetClearColor();
    bool renderWireframe = m_Context->GetRenderWireframe();

    ImGui::SetNextWindowPos(ImVec2(20, 20));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(400, 0));
    {
        ImGui::Begin("Debug Menu", nullptr, GetWindowFlags());
        ImGui::Text("Ether Version: 0.1.0");
        ImGui::Spacing();

        if (ImGui::CollapsingHeader("Display Options"))
        {
            ImGui::ColorEdit3("clear color", (float*)& clearColor); // Edit 3 floats representing a color
            ImGui::Checkbox("Render Wireframe", &renderWireframe);      // Edit bools storing our window open/close state
        }

        if (ImGui::CollapsingHeader("ImGui"))
        {
            ImGui::Checkbox("Show ImGui Demo Window", &showImGuiDemo);      // Edit bools storing our window open/close state
        }

        ImGui::Text("Frame Time: %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::End();

        m_Context->SetClearColor(clearColor);
        m_Context->SetRenderWireframe(renderWireframe);
    }
    ImGui::PopStyleVar();

    if (showImGuiDemo)
    {
        ImGui::ShowDemoWindow(&showImGuiDemo);
    }
}

ImGuiWindowFlags GfxImGui::GetWindowFlags() const
{
    ImGuiWindowFlags windowFlags = 0;

    windowFlags |= ImGuiWindowFlags_NoMove;
    windowFlags |= ImGuiWindowFlags_NoResize;
    windowFlags |= ImGuiWindowFlags_AlwaysAutoResize;
    windowFlags |= ImGuiWindowFlags_NoCollapse;

    return windowFlags;
}

