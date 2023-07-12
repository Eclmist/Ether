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
#include "graphics/rhi/rhiimguiwrapper.h"
#include "graphics/imgui/imgui.h"
#include "graphics/rhi/dx12/dx12imguiwrapper.h"

Ether::Graphics::RhiImguiWrapper::RhiImguiWrapper()
    : m_Context("Imgui Context")
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    SetStyle();
}

Ether::Graphics::RhiImguiWrapper::~RhiImguiWrapper()
{
    ImGui::DestroyContext();
}

void Ether::Graphics::RhiImguiWrapper::Render()
{
    ImGui::NewFrame();

    ETH_MARKER_EVENT("Debug Menu Gui Component - Draw");

    static bool showImGuiDemo = false;
    auto& gfxConfig = GraphicCore::GetGraphicConfig();

    ImGui::SetNextWindowPos(ImVec2(20, 20));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(400, 0));
    {
        ImGui::Begin(
            "Debug Menu",
            nullptr,
            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize |
                ImGuiWindowFlags_NoCollapse);

        ImGui::Text("Ether Version: 0.1.0");
        ImGui::Spacing();

        if (ImGui::CollapsingHeader("Display Options"))
        {
            static bool vSync = GraphicCore::GetGraphicDisplay().IsVsyncEnabled();
            ImGui::Checkbox("VSync Enabled", &vSync);
            GraphicCore::GetGraphicDisplay().SetVSyncEnabled(vSync);

            static int numVblanks = 1;
            if (vSync)
            {
                ImGui::SameLine(180);
                ImGui::PushItemWidth(100);
                ImGui::InputInt("Num V-Blanks", &numVblanks, 1, 1);
                ImGui::PopItemWidth();
                numVblanks = std::clamp(numVblanks, 1, 4);
                GraphicCore::GetGraphicDisplay().SetVSyncVBlanks(numVblanks);
            }
        }

        if (ImGui::CollapsingHeader("Render Options"))
        {
            static float clearColor[4];
            ImGui::ColorEdit4("clear color", clearColor);
            // Edit 3 floats representing a color
            // ImGui::Checkbox("Render Wireframe", &EngineCore::GetEngineConfig().m_RenderWireframe);
            // Edit bools storing our window open/close state
            gfxConfig.SetClearColor((ethVector4&)(*clearColor));

            static bool& temporalAAEnabled = gfxConfig.m_IsTemporalAAEnabled;
            ImGui::Checkbox("Temporal AA", &temporalAAEnabled);

            if (temporalAAEnabled)
            {
                const char* items[] = { "None", "Grid", "Halton" };
                ImGui::Combo("Jitter Mode", &gfxConfig.m_TemporalAAJitterMode, items, IM_ARRAYSIZE(items));
                ImGui::SliderFloat("Jitter Scale (debug)", &gfxConfig.m_JitterScale, 0, 10);
                ImGui::SliderFloat("Temporal Accumulation", &gfxConfig.m_TemporalAAAcumulationFactor, 0, 1);
            }

            static bool& raytracingEnabled = gfxConfig.m_IsRaytracingEnabled;
            ImGui::Checkbox("Raytracing", &raytracingEnabled);
            if (raytracingEnabled)
            {
                ImGui::SliderFloat("AO Intensity", &gfxConfig.m_RaytracedAOIntensity, 0, 1);
            }

            static ethVector4& sunColor = gfxConfig.m_SunColor;
            static ethVector4& sunDirection = gfxConfig.m_SunDirection;
            ImGui::ColorEdit3("Sun Color", sunColor.m_Data);
            ImGui::SliderFloat3("Sun Direction", sunDirection.m_Data, -1, 1);

            sunDirection.Normalize();
        }

        static float fpsHistoryBuffer[128];
        fpsHistoryBuffer[ImGui::GetFrameCount() % 128] = ImGui::GetIO().Framerate;

        if (ImGui::CollapsingHeader("Performance"))
        {
            ImGui::Text(
                "Frame Time: %.3f ms/frame (%.1f FPS)",
                1000.0f / ImGui::GetIO().Framerate,
                ImGui::GetIO().Framerate);
            ImGui::Text("Frame Number: %lld", Graphics::GraphicCore::GetGraphicRenderer().GetFrameNumber());
            ImGui::PlotLines(
                "",
                fpsHistoryBuffer,
                128,
                ImGui::GetFrameCount() % 128,
                "FPS History",
                0.0f,
                300.0f,
                ImVec2(360, 60));
        }
        ImGui::End();
    }
    ImGui::PopStyleVar();

    if (showImGuiDemo)
    {
        ImGui::ShowDemoWindow(&showImGuiDemo);
    }

    ImGui::Render();

    m_Context.Reset();
    m_Context.TransitionResource(GraphicCore::GetGraphicDisplay().GetBackBuffer(), RhiResourceState::RenderTarget);
    m_Context.SetRenderTarget(GraphicCore::GetGraphicDisplay().GetBackBufferRtv());
    m_Context.SetSrvCbvUavDescriptorHeap(*m_DescriptorHeap);
    m_Context.SetGraphicRootSignature(*GraphicCore::GetGraphicCommon().m_EmptyRootSignature);

    RenderDrawData();
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

void Ether::Graphics::RhiImguiWrapper::SetStyle() const
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
