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

#include "loggingmanager.h"
#include <iosfwd>
#include <ShlObj_core.h>
#include <iostream>

ETH_NAMESPACE_BEGIN

void Log(LogLevel level, LogType type, const char* fmt, ...)
{
    char formattedBuffer[4096];

    va_list args;
    va_start(args, fmt);
    vsprintf_s(formattedBuffer, fmt, args);
    va_end(args);

    std::string formattedText(formattedBuffer);
    LogEntry entry(formattedText, level, type);
    g_LoggingManager.AddLog(entry);
    g_LoggingManager.Serialize(entry);
    std::cout << entry.GetText() << std::endl;
}

void LoggingManager::Initialize()
{
    m_LogFileStream.open(std::wstring(GetOutputDirectory() + L"/" + GetTimestampedFileName()), std::ios_base::app);

    if (!m_LogFileStream.is_open())
    {
        LogInfo("Unable to open log file for serialization. Logs may be lost.\n");
        return;
    }
}

void LoggingManager::Shutdown()
{
    m_LogFileStream.close();
}

void LoggingManager::AddLog(const LogEntry entry)
{
    m_LogEntries.push_back(entry);
}

void LoggingManager::Clear()
{
    m_LogEntries.clear();
}

void LoggingManager::Serialize(const LogEntry entry)
{
    if (!m_LogFileStream.is_open())
        return;

    m_LogFileStream << entry.GetText().c_str() << "\n";
    m_LogFileStream.flush();
}

const std::wstring LoggingManager::GetOutputDirectory() const
{
    wchar_t path[1024];
    wchar_t* appDataLocal;
    SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &appDataLocal);
    wcscpy_s(path, appDataLocal);
    wcscat_s(path, L"\\Ether");
    CreateDirectory(path, NULL);
    wcscat_s(path, L"\\");
    wcscat_s(path, g_EngineConfig.GetClientName().c_str());
    CreateDirectory(path, NULL);
    wcscat_s(path, L"\\Logs");
    CreateDirectory(path, NULL);
    return path;
}

const std::wstring LoggingManager::GetTimestampedFileName() const
{
    wchar_t filename[1024];
    wcscpy_s(filename, WFormatTime(GetSystemTime(), L"%Y%m%d_%H%M%S").c_str());
    wcscat_s(filename, L".log");
    return filename;
}

ETH_NAMESPACE_END

