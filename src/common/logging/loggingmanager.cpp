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

#include "common/logging/loggingmanager.h"
#include "common/time/time.h"

#include <stdarg.h>
#include <sstream>
#include <filesystem>

#ifdef ETH_PLATFORM_WIN32
#define _AMD64_ // So we don't have to include <windows.h> here
#include <debugapi.h> // For OutputDebugStringA
#endif

constexpr uint32_t MaxLogEntries = 512;
constexpr std::string_view OutputFilePath = ".\\Logs";
constexpr std::string_view OutputFileSuffix = ".log";

Ether::LoggingManager::~LoggingManager()
{
    m_LogFileStream.close();
}

void Ether::LoggingManager::Initialize()
{
#if defined (ETH_PLATFORM_WIN32)
    std::filesystem::create_directory(OutputFilePath);
    m_LogFileStream.open(GetOutputDirectory() + "/" + GetTimestampedFileName(), std::ios_base::app);

    if (!m_LogFileStream.is_open())
        LogWarning("Unable to open log file for serialization.Logs will be lost when the application exits");
#endif
}

void Ether::LoggingManager::Log(LogLevel level, LogType type, const char* fmt, ...)
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

        AddLog(entry);
        Serialize(entry);
    }
}

void Ether::LoggingManager::AddLog(const LogEntry entry)
 {
    std::lock_guard<std::mutex> lock(m_Mutex);

    if (m_LogEntries.size() > MaxLogEntries)
        m_LogEntries.pop_front();

    m_LogEntries.emplace_back(entry);
}

void Ether::LoggingManager::Serialize(const LogEntry entry)
{
    std::lock_guard<std::mutex> lock(m_Mutex);

#if defined (ETH_PLATFORM_WIN32)
    m_LogFileStream << entry.GetFullText().c_str() << "\n";
    m_LogFileStream.flush();

    // TODO: Also output the log entry to visual studio's debug output window
    OutputDebugStringA((entry.GetFullText() + "\n").c_str());

#elif defined (ETH_PLATFORM_PS5)
    fprintf((&_CSTD _Stdout), "%s\n", entry.GetFullText().c_str());
#endif
}

void Ether::LoggingManager::Clear()
{
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_LogEntries.clear();
}

const std::string Ether::LoggingManager::GetOutputDirectory() const
{
    return std::string(OutputFilePath);
}

const std::string Ether::LoggingManager::GetTimestampedFileName() const
{
    return std::to_string(Time::GetStartupTime()) + std::string(OutputFileSuffix);
}

