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
#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx12.h"

GfxImGui::GfxImGui()
    : m_IsVisible(false)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
}

void GfxImGui::Initialize(DX12Device* device, DX12DescriptorHeap* srvDescriptor) const
{
    ImGui_ImplDX12_Init(
        device->Get().Get(),
        ETH_NUM_SWAPCHAIN_BUFFERS,
        DXGI_FORMAT_R8G8B8A8_UNORM,
        srvDescriptor->Get().Get(),
        srvDescriptor->Get()->GetCPUDescriptorHandleForHeapStart(),
        srvDescriptor->Get()->GetGPUDescriptorHandleForHeapStart()
    );
}

void GfxImGui::Render(DX12CommandList* commandList) const
{
    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    SetupUI();

    ImGui::Render();
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList->Get().Get());
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
    bool show_another_window = false;
    static bool show_demo_window = true;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    if (show_demo_window)
    {
        ImGui::ShowDemoWindow(&show_demo_window);
    }

    // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
    {
        static float f = 0.0f;
        static int counter = 0;

        ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

        ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
        ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
        ImGui::Checkbox("Another Window", &show_another_window);

        ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
        ImGui::ColorEdit3("clear color", (float*)& clear_color); // Edit 3 floats representing a color

        if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
            counter++;
        ImGui::SameLine();
        ImGui::Text("counter = %d", counter);

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::End();
    }

    // 3. Show another simple window.
    if (show_another_window)
    {
        ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
        ImGui::Text("Hello from another window!");
        if (ImGui::Button("Close Me"))
            show_another_window = false;
        ImGui::End();
    }
}

