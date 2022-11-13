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

#include "guirenderer.h"

#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx12.h"
#include "graphic/gui/loggingguicomponent.h"
#include "graphic/gui/debugmenuguicomponent.h"

#include "graphic/rhi/dx12/dx12commandlist.h"
#include "graphic/rhi/dx12/dx12device.h"
#include "graphic/rhi/dx12/dx12descriptorheap.h"
#include "graphic/rhi/rhidescriptorheap.h"
#include "graphic/rhi/rhicommandlist.h"

ETH_NAMESPACE_BEGIN

GuiRenderer::GuiRenderer()
    : m_Context(L"GUIRendererContext")
{
    LogGraphicsInfo("Initializing GUI Manager");

    const auto d3dDevice = GraphicCore::GetDevice().As<Dx12Device>();

    m_SrvDescriptorHeap.SetName(L"GUIRenderer::SRVDescriptorHeap");
    GraphicCore::GetDevice()->CreateDescriptorHeap({ RhiDescriptorHeapType::CbvSrvUav, RhiDescriptorHeapFlag::ShaderVisible, 4096 }, m_SrvDescriptorHeap);

    RegisterComponents();
    CreateImGuiContext();
    ImGui_ImplWin32_Init(EngineCore::GetMainWindow().GetWindowHandle());
    ImGui_ImplDX12_Init(
        d3dDevice->m_Device.Get(),
        3, // ETH_MAX_NUM_SWAPCHAIN_BUFFERS,
        DXGI_FORMAT_R8G8B8A8_UNORM,
        m_SrvDescriptorHeap.As<Dx12DescriptorHeap>()->m_Heap.Get(),
        m_SrvDescriptorHeap.As<Dx12DescriptorHeap>()->m_Heap->GetCPUDescriptorHandleForHeapStart(),
        m_SrvDescriptorHeap.As<Dx12DescriptorHeap>()->m_Heap->GetGPUDescriptorHandleForHeapStart()
    );

    SetImGuiStyle();
}

GuiRenderer::~GuiRenderer()
{
    ImGui_ImplDX12_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

void GuiRenderer::Render()
{
    ETH_MARKER_EVENT("Gui Renderer - Render");

    if (m_Components.empty())
        return;

    if (!EngineCore::GetEngineConfig().IsDebugGuiEnabled())
        return;

    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    for (auto&& component : m_Components)
        component->Draw();

    ImGui::Render();
    m_Context.TransitionResource(GraphicCore::GetGraphicDisplay().GetCurrentBackBuffer(), RhiResourceState::RenderTarget);
    m_Context.SetRenderTarget(GraphicCore::GetGraphicDisplay().GetCurrentBackBufferRtv());

    
    m_Context.GetCommandList()->SetDescriptorHeaps({ 1, &m_SrvDescriptorHeap });
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), m_Context.GetCommandList().As<Dx12CommandList>()->m_CommandList.Get());
    m_Context.FinalizeAndExecute();
    m_Context.Reset();
}

void GuiRenderer::RegisterComponents()
{
    m_Components.push_back(std::make_unique<LoggingGuiComponent>());
    m_Components.push_back(std::make_unique<DebugMenuGuiComponent>());
}

void GuiRenderer::CreateImGuiContext()
{
    LogGraphicsInfo("Initializing ImGui v%s", IMGUI_VERSION);
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
}

void GuiRenderer::SetImGuiStyle()
{
    ImGuiStyle* style = &ImGui::GetStyle();
    style->WindowBorderSize = 0.0f;
    style->FrameBorderSize = 0.0f;
    style->PopupBorderSize = 0.0f;
    style->TabBorderSize = 0.0f;
    style->ChildBorderSize = 0.0f;
    style->WindowPadding = ImVec2(20, 15);
    style->WindowRounding = 5.0f;
    style->FramePadding = ImVec2(5, 5);
    style->FrameRounding = 4.0f;
    style->ItemSpacing = ImVec2(12, 8);
    style->ItemInnerSpacing = ImVec2(8, 6);
    style->IndentSpacing = 25.0f;
    style->ScrollbarSize = 15.0f;
    style->ScrollbarRounding = 9.0f;
    style->GrabMinSize = 5.0f;
    style->GrabRounding = 0.0f;
    style->Alpha = 0.96f;

    style->Colors[ImGuiCol_Text] = ImVec4(0.80f, 0.80f, 0.83f, 1.00f);
    style->Colors[ImGuiCol_TextDisabled] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
    style->Colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
    style->Colors[ImGuiCol_PopupBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
    style->Colors[ImGuiCol_Border] = ImVec4(0.80f, 0.80f, 0.83f, 0.88f);
    style->Colors[ImGuiCol_BorderShadow] = ImVec4(0.92f, 0.91f, 0.88f, 0.00f);
    style->Colors[ImGuiCol_FrameBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
    style->Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
    style->Colors[ImGuiCol_FrameBgActive] = ImVec4(0.36f, 0.36f, 0.38f, 1.00f);
    style->Colors[ImGuiCol_TitleBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
    style->Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.10f, 0.10f, 0.12f, 1.00);
    style->Colors[ImGuiCol_TitleBgActive] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
    style->Colors[ImGuiCol_MenuBarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
    style->Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
    style->Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
    style->Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
    style->Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
    style->Colors[ImGuiCol_CheckMark] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
    style->Colors[ImGuiCol_SliderGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
    style->Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
    style->Colors[ImGuiCol_Button] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
    style->Colors[ImGuiCol_ButtonHovered] = ImVec4(0.34f, 0.33f, 0.39f, 1.00f);
    style->Colors[ImGuiCol_ButtonActive] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
    style->Colors[ImGuiCol_Header] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
    style->Colors[ImGuiCol_HeaderActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
    style->Colors[ImGuiCol_HeaderHovered] = ImVec4(0.34f, 0.33f, 0.39f, 1.00f);
    style->Colors[ImGuiCol_ResizeGrip] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    style->Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
    style->Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
    style->Colors[ImGuiCol_PlotLines] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
    style->Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
    style->Colors[ImGuiCol_PlotHistogram] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
    style->Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
    style->Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.25f, 1.00f, 0.00f, 0.43f);
    style->Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(1.00f, 0.98f, 0.95f, 0.73f);
    style->Colors[ImGuiCol_Tab] = style->Colors[ImGuiCol_Button];
    style->Colors[ImGuiCol_TabHovered] = style->Colors[ImGuiCol_ButtonActive];
    style->Colors[ImGuiCol_TabActive] = style->Colors[ImGuiCol_ButtonHovered];
}

ETH_NAMESPACE_END

