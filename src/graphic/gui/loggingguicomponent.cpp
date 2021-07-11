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

#include "loggingguicomponent.h"

ETH_NAMESPACE_BEGIN

LoggingGuiComponent::LoggingGuiComponent()
{
    m_Size = ImVec2((float) g_EngineConfig.GetClientWidth(), 300);
    m_Position = ImVec2(0, g_EngineConfig.GetClientHeight() - m_Size.y);
}

void LoggingGuiComponent::Draw()
{
    ImGui::SetNextWindowSize(m_Size, ImGuiCond_Once);
    ImGui::SetNextWindowPos(m_Position, ImGuiCond_Once);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 1.0);
    ImGui::Begin("Debug Output", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
    ImGui::BeginChild("scrolling", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));

    auto logEntries = g_LoggingManager.GetEntries();

    ImGuiListClipper clipper;
    clipper.Begin((int)logEntries.size());
    while (clipper.Step())
    {
        for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i)
        {
            ImGui::TextColored(GetColor(logEntries[i].m_Level), logEntries[i].GetText().c_str());
        }
    }
    clipper.End();

    ImGui::PopStyleVar();
    ImGui::PopStyleVar();

    // Auto scroll
    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
        ImGui::SetScrollHereY(1.0f);

    ImGui::EndChild();
    ImGui::End();
}

ImVec4 LoggingGuiComponent::GetColor(LogLevel level) const
{
    switch (level)
    {
    case LogLevel::LOGLEVEL_INFO:
        return ImVec4(1, 1, 1, 1);
        break;
    case LogLevel::LOGLEVEL_WARNING:
        return ImVec4(1, 1, 0, 1);
    case LogLevel::LOGLEVEL_ERROR:
        return ImVec4(1, 0.4f, 0.4f, 1);
    case LogLevel::LOGLEVEL_FATAL:
        return ImVec4(1, 0.2f, 0.2f, 1);
    default:
        return ImVec4(1, 1, 1, 1);
    }
}

ETH_NAMESPACE_END