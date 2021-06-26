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

#include "gfximgui.h"
#include "graphic/hal/dx12commandlist.h"
#include "graphic/hal/dx12descriptorheap.h"
#include "graphic/hal/dx12device.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx12.h"

ETH_NAMESPACE_BEGIN

GfxImGui::GfxImGui()
    : m_IsVisible(false)
    , m_Context(nullptr)
    , m_FpsHistoryOffset(0)
{
    CreateImGuiContext();
    ImGui_ImplWin32_Init(Win32::g_hWnd);
    SetStyle();
}

void GfxImGui::CreateImGuiContext()
{
}

void GfxImGui::SetStyle()
{
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

void GfxImGui::Render(DX12CommandList& commandList)
{
    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    UpdateFpsHistory();
    SetupDebugMenu();
    SetupGuiComponents();

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

void GfxImGui::UpdateFpsHistory()
{
    static float refresh_time = 0;
    while (refresh_time < ImGui::GetTime()) // Create data at fixed 60 Hz rate for the demo
    {
        refresh_time += 10.0f / 60.0f;
        m_FpsHistory[m_FpsHistoryOffset] = ImGui::GetIO().Framerate;
        m_FpsHistoryOffset = (m_FpsHistoryOffset + 1) % HistoryBufferSize;
    }
}

void GfxImGui::SetupDebugMenu() const
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

        if (ImGui::CollapsingHeader("Performance"))
        {
            ImGui::Text("Frame Time: %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::PlotLines("", m_FpsHistory, HistoryBufferSize, m_FpsHistoryOffset, nullptr, 0.0f, 300.0f, ImVec2(384, 100));
        }
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

ETH_NAMESPACE_END

