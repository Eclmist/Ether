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

void Log(LogLevel level, LogType type, const char* fmt, ...)
{

    char formattedBuffer[4096];

    va_list args;
    va_start(args, fmt);
    vsprintf_s(formattedBuffer, fmt, args);
    va_end(args);

    std::string formattedText(formattedBuffer);
    Logger::GetInstance().AddLog(LogEntry(formattedText, level, type));
}


Logger::Logger()
{
    Clear();
}

void Logger::AddLog(const LogEntry&& entry)
{
    m_LogEntries.push_back(entry);
}

void Logger::Clear()
{
    m_LogEntries.clear();
}

void Logger::DrawImGui()
{
    ImGui::SetNextWindowSize(ImVec2(1920, 300));
    ImGui::SetNextWindowPos(ImVec2(0, 1080 - 300));

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 1.0);
    ImGui::Begin("Debug Output", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
    ImGui::BeginChild("scrolling", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));

    ImGuiListClipper clipper;
    clipper.Begin((int)m_LogEntries.size());
    while (clipper.Step())
    {
        for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; ++i)
        {
            ImGui::TextColored(m_LogEntries[i].GetColor(), m_LogEntries[i].GetText().c_str());
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

void Logger::Serialize()
{
    std::wfstream outfile;
    outfile.open(std::wstring(GetOutputDirectory() + L"/" + GetTimestampedFileName()), std::ios_base::app);

    if (!outfile.is_open()) 
    {
        LogInfo("Unable to open log file for serialization. Logs may be lost.\n");
        return;
    }

    for (int i = 0; i < m_LogEntries.size(); ++i)
        outfile << m_LogEntries[i].GetText().c_str() << '\n';

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
    wcscpy_s(filename, WFormatTime(GetSystemTime(), L"%Y%m%d%H%M%S").c_str()); // TODO: Replace with start time
    wcscat_s(filename, L".log");
    return filename;
}

LogEntry::LogEntry(const std::string& text, LogLevel level, LogType type)
    : m_Text(text)
    , m_Level(level)
    , m_Type(type)
{
    m_Time = GetSystemTime();
}

ImVec4 LogEntry::GetColor() const
{
    switch (m_Level)
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

std::string LogEntry::GetText() const
{
    return GetTimePrefix() + " " + m_Text;
}

std::string LogEntry::GetLogLevelPrefix() const
{
    switch (m_Level)
    {
    case LogLevel::LOGLEVEL_INFO:
        return "[  INFO   ]";
    case LogLevel::LOGLEVEL_WARNING:
        return "[ WARNING ]";
    case LogLevel::LOGLEVEL_ERROR:
        return "[  ERROR  ]";
    case LogLevel::LOGLEVEL_FATAL:
        return "[  FATAL  ]";
    default:
        LogError("Invalid LOGLEVEL encountered by logger");
        return "";
    }
}

std::string LogEntry::GetLogTypePrefix() const
{
    switch (m_Type)
    {
    case LogType::LOGTYPE_ENGINE:
        return "Engine:";
    case LogType::LOGTYPE_GRAPHICS:
        return "Graphics:";
    case LogType::LOGTYPE_WIN32:
        return "Win32:";
    case LogType::LOGTYPE_NONE:
    default:
        return "";
    }
}

std::string LogEntry::GetTimePrefix() const
{
    return "[" + FormatTime(m_Time) + "]";
}

ETH_NAMESPACE_END
