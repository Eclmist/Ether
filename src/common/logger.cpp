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

#include "logger.h"
#include <iosfwd>
#include <fstream>
#include <ShlObj_core.h>

ETH_NAMESPACE_BEGIN

#define vLOG(a)\
    va_list args;\
    va_start(args, a);\
    GetInstance().Log(a, args);\
    va_end(args);

Logger::Logger()
{
    Clear();
}

void Logger::LogInfo(const char* fmt, ...)
{
    GetInstance().m_Buffer.append("[INFO] ");
    vLOG(fmt);
}

void Logger::LogWarning(const char* fmt, ...)
{
    GetInstance().m_Buffer.append("[WARNING] ");
    vLOG(fmt);
}

void Logger::LogError(const char* fmt, ...)
{
    GetInstance().m_Buffer.append("[ERROR] ");
    vLOG(fmt);
}

void Logger::LogFatal(const char* fmt, ...)
{
    GetInstance().m_Buffer.append("[FATAL] ");
    vLOG(fmt);
}

void Logger::Log(const char* fmt, va_list args)
{
    int oldSize = m_Buffer.size();
    m_Buffer.appendfv(fmt, args);

    if (m_Buffer[m_Buffer.size()  - 1] != '\n')
        m_Buffer.append("\n");

    for (int newSize = m_Buffer.size(); oldSize < newSize; oldSize++)
        if (m_Buffer[oldSize] == '\n')
            m_LineOffsets.push_back(oldSize + 1);
}

void Logger::Clear()
{
    m_Buffer.clear();
    m_LineOffsets.clear();
    m_LineOffsets.push_back(0);
}

void Logger::DrawImGui()
{
    ImGui::SetNextWindowSize(ImVec2(1920, 300));
    ImGui::SetNextWindowPos(ImVec2(0, 1080 - 300));

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 1.0);
    ImGui::Begin("Debug Output", nullptr, ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));

    const char* bufStart = m_Buffer.begin();
    const char* bufEnd = m_Buffer.end();

    ImGuiListClipper clipper;
    clipper.Begin((int)m_LineOffsets.size());
    while (clipper.Step())
    {
        for (int lineNum = clipper.DisplayStart; lineNum < clipper.DisplayEnd; ++lineNum)
        {
            const char* lineStart = bufStart + m_LineOffsets[lineNum];
            const char* lineEnd = (lineNum + 1 < m_LineOffsets.size() ? (bufStart + m_LineOffsets[lineNum + 1] - 1) : bufEnd);
            ImGui::TextUnformatted(lineStart, lineEnd);
        }
    }
    clipper.End();

    ImGui::PopStyleVar();
    ImGui::PopStyleVar();

    // Auto scroll
    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
        ImGui::SetScrollHereY(1.0f);

    ImGui::End();
}

void Logger::Serialize()
{
    std::wfstream outfile;
    outfile.open(std::wstring(GetOutputDirectory() + L"/" + GetTimestampedFileName()), std::ios_base::app);

    if (!outfile.is_open()) 
    {
        LogWarning("Unable to open log file for serialization. Logs may be lost.\n");
        return;
    }

    outfile << m_Buffer.Buf.Data;
    outfile.close();
}

const std::wstring Logger::GetOutputDirectory() const
{
    wchar_t path[1024];
    wchar_t* appDataLocal;
    SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &appDataLocal);
    wcscpy_s(path, appDataLocal);
    wcscat_s(path, L"\\Ether");
    CreateDirectory(path, NULL);
    wcscat_s(path, L"\\ApplicationName"); //TODO: Replace with current app name
    CreateDirectory(path, NULL);
    wcscat_s(path, L"\\Logs");
    CreateDirectory(path, NULL);
    return path;
}

const std::wstring Logger::GetTimestampedFileName() const
{
    wchar_t filename[1024];
    wcscpy_s(filename, L"YYYYMMDD_HHMMSS"); // TODO: Replace with current time
    wcscat_s(filename, L".log");
    return filename;
}

ETH_NAMESPACE_END
