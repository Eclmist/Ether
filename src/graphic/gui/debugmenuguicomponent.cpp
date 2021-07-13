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

#include "debugmenuguicomponent.h"

ETH_NAMESPACE_BEGIN

DebugMenuGuiComponent::DebugMenuGuiComponent()
{
    m_Size = ImVec2((float)EngineCore::GetEngineConfig().GetClientWidth(), 300);
    m_Position = ImVec2(0, EngineCore::GetEngineConfig().GetClientHeight() - m_Size.y);
}

void DebugMenuGuiComponent::Draw()
{
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
        }

        if (ImGui::CollapsingHeader("Input"))
        {
            ImGui::Text("Mouse Position: (%f, %f)", Input::GetMousePosX(), Input::GetMousePosY());
            ImGui::Text("Mouse Delta: (%f, %f)", Input::GetMouseDeltaX(), Input::GetMouseDeltaY());
        }

        if (ImGui::CollapsingHeader("Scene"))
        {
            for (auto entity : EngineCore::GetActiveWorld().GetEntities())
            {
                ImGui::BulletText(entity->GetName().c_str());
            }

            if (ImGui::Button("Add entity"))
            {
                Entity* newEntity = new Entity("Entity");
                newEntity->GetTransform()->SetPosition({ (float)(rand() % 100 - 50), (float)(rand() % 100 - 50), (float)(rand() % 100 - 50)});
                newEntity->GetTransform()->SetRotation({ (float)rand(), (float)rand(), (float)rand() });
                newEntity->AddComponent<VisualComponent>();
                EngineCore::GetActiveWorld().AddEntity(newEntity);
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
