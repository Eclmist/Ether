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
#include <sstream>

ETH_NAMESPACE_BEGIN

#define MAX_LOG_ENTRIES 512

std::queue<LogEntry> g_PreInitBuffer;

void Log(LogLevel level, LogType type, const char* fmt, ...)
{
    char formattedBuffer[4096];

    va_list args;
    va_start(args, fmt);
    vsprintf_s(formattedBuffer, fmt, args);
    va_end(args);

    std::string formattedText(formattedBuffer);

    std::stringstream ss(formattedText);
    std::string individualLine;

    while (std::getline(ss, individualLine, '\n')) {
        LogEntry entry(individualLine, level, type);

        if (EngineCore::HasInstance())
        {
            EngineCore::GetLoggingManager().AddLog(entry);
            EngineCore::GetLoggingManager().Serialize(entry);
        }
        else
            g_PreInitBuffer.push(entry);
    }
}

LoggingManager::LoggingManager()
{
    m_LogFileStream.open(std::wstring(GetOutputDirectory() + GetTimestampedFileName()), std::ios_base::app);

    if (!m_LogFileStream.is_open())
    {
        // Warning must push to the pre-init buffer manually instead of calling any of the
        // Logging functions because the logging manager is in the middle of initialization.
        g_PreInitBuffer.emplace(
            "Unable to open log file for serialization. Logs may be lost",
            LogLevel::LOGLEVEL_WARNING, LogType::LOGTYPE_ENGINE);

        return;
    }

    while (!g_PreInitBuffer.empty())
    {
        AddLog(g_PreInitBuffer.front());
        Serialize(g_PreInitBuffer.front());
        g_PreInitBuffer.pop();
    }
}

LoggingManager::~LoggingManager()
{
    m_LogFileStream.close();
}

void LoggingManager::AddLog(const LogEntry entry)
 {
    std::lock_guard<std::mutex> guard(m_Mutex);

    if (m_LogEntries.size() > MAX_LOG_ENTRIES)
        m_LogEntries.pop_front();

    m_LogEntries.emplace_back(entry);
}

void LoggingManager::Clear()
{
    m_LogEntries.clear();
}

void LoggingManager::Serialize(const LogEntry entry)
{
    std::lock_guard<std::mutex> guard(m_Mutex);
    m_LogFileStream << entry.GetText().c_str() << "\n";
    m_LogFileStream.flush();
}

const std::wstring LoggingManager::GetOutputDirectory() const
{
    wchar_t path[1024];
    wcscpy_s(path, L"./Logs");
    CreateDirectory(path, NULL);
    wcscat_s(path, L"/");
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

