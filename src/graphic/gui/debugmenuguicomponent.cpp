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

#include "debugmenuguicomponent.h"

ETH_NAMESPACE_BEGIN

DebugMenuGuiComponent::DebugMenuGuiComponent()
{
    m_Size = ImVec2((float)EngineCore::GetEngineConfig().GetClientWidth(), 300);
    m_Position = ImVec2(0, EngineCore::GetEngineConfig().GetClientHeight() - m_Size.y);
}

void DebugMenuGuiComponent::Draw()
{
    OPTICK_EVENT("Debug Menu Gui Component - Draw");

    static bool showImGuiDemo = false;
    GraphicContext& gfxContext = GraphicCore::GetGraphicRenderer().GetGraphicContext();

    ImGui::SetNextWindowPos(ImVec2(20, 20));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(400, 0));
    {
        ImGui::Begin("Debug Menu", nullptr,
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_AlwaysAutoResize |
            ImGuiWindowFlags_NoCollapse);

        ImGui::Text("Ether Version: 0.1.0");
        ImGui::Spacing();

        if (ImGui::CollapsingHeader("Display Options"))
        {
            ImGui::ColorEdit3("clear color", (float*)&EngineCore::GetEngineConfig().m_ClearColor); // Edit 3 floats representing a color
            ImGui::Checkbox("Render Wireframe", &EngineCore::GetEngineConfig().m_RenderWireframe);      // Edit bools storing our window open/close state

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

        if (ImGui::CollapsingHeader("Input"))
        {
            ImGui::Text("Mouse Position: (%f, %f)", Input::GetMousePosX(), Input::GetMousePosY());
            ImGui::Text("Mouse Delta: (%f, %f)", Input::GetMouseDeltaX(), Input::GetMouseDeltaY());
        }

        if (ImGui::CollapsingHeader("Scene"))
        {
            for (int i = 0; i < ETH_ECS_MAX_ENTITIES; ++i)
            {
                auto entity = EngineCore::GetEcsManager().GetEntity(i);
                if (entity != nullptr)
                    ImGui::BulletText(entity->GetName().c_str());
            }

            if (ImGui::Button("Add entity"))
            {
                Entity* newEntity = EngineCore::GetEcsManager().CreateEntity("New Entity");
                newEntity->GetComponent<TransformComponent>()->SetPosition({ (float)(rand() % 100 - 50), (float)(rand() % 100 - 50), (float)(rand() % 100 - 50)});
                newEntity->GetComponent<TransformComponent>()->SetRotation({ (float)rand(), (float)rand(), (float)rand() });
                newEntity->AddComponent<MeshComponent>();
                newEntity->AddComponent<VisualComponent>();
            }
        }

        if (ImGui::CollapsingHeader("ImGui"))
        {
            ImGui::Checkbox("Show ImGui Demo Window", &showImGuiDemo);      // Edit bools storing our window open/close state
        }

        if (ImGui::CollapsingHeader("Performance"))
        {
            ImGui::Text("Frame Time: %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            //ImGui::PlotLines("", m_FpsHistory, HistoryBufferSize, m_FpsHistoryOffset, nullptr, 0.0f, 300.0f, ImVec2(384, 100));
        }
        ImGui::End();

        //gfxContext->SetClearColor(clearColor);
        //gfxContext->SetRenderWireframe(renderWireframe);
    }
    ImGui::PopStyleVar();

    if (showImGuiDemo)
    {
        ImGui::ShowDemoWindow(&showImGuiDemo);
    }
}

ETH_NAMESPACE_END
