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

#pragma once

#include "logentry.h"
#include <fstream>

ETH_NAMESPACE_BEGIN

#define LogInfo(msg, ...) Ether::Log(Ether::LogLevel::LOGLEVEL_INFO, Ether::LogType::LOGTYPE_NONE, msg, ##__VA_ARGS__)
#define LogWarning(msg, ...) Ether::Log(Ether::LogLevel::LOGLEVEL_WARNING, Ether::LogType::LOGTYPE_NONE, msg, ##__VA_ARGS__)
#define LogError(msg, ...) Ether::Log(Ether::LogLevel::LOGLEVEL_ERROR, Ether::LogType::LOGTYPE_NONE, msg, ##__VA_ARGS__)
#define LogFatal(msg, ...) Ether::Log(Ether::LogLevel::LOGLEVEL_FATAL, Ether::LogType::LOGTYPE_NONE, msg, ##__VA_ARGS__)

#define LogEngineInfo(msg, ...) Ether::Log(Ether::LogLevel::LOGLEVEL_INFO, Ether::LogType::LOGTYPE_ENGINE, msg, ##__VA_ARGS__)
#define LogEngineWarning(msg, ...) Ether::Log(Ether::LogLevel::LOGLEVEL_WARNING, Ether::LogType::LOGTYPE_ENGINE, msg, ##__VA_ARGS__)
#define LogEngineError(msg, ...) Ether::Log(Ether::LogLevel::LOGLEVEL_ERROR, Ether::LogType::LOGTYPE_ENGINE, msg, ##__VA_ARGS__)
#define LogEngineFatal(msg, ...) Ether::Log(Ether::LogLevel::LOGLEVEL_FATAL, Ether::LogType::LOGTYPE_ENGINE, msg, ##__VA_ARGS__)

#define LogGraphicsInfo(msg, ...) Ether::Log(Ether::LogLevel::LOGLEVEL_INFO, Ether::LogType::LOGTYPE_GRAPHICS, msg, ##__VA_ARGS__)
#define LogGraphicsWarning(msg, ...) Ether::Log(Ether::LogLevel::LOGLEVEL_WARNING, Ether::LogType::LOGTYPE_GRAPHICS, msg, ##__VA_ARGS__)
#define LogGraphicsError(msg, ...) Ether::Log(Ether::LogLevel::LOGLEVEL_ERROR, Ether::LogType::LOGTYPE_GRAPHICS, msg, ##__VA_ARGS__)
#define LogGraphicsFatal(msg, ...) Ether::Log(Ether::LogLevel::LOGLEVEL_FATAL, Ether::LogType::LOGTYPE_GRAPHICS, msg, ##__VA_ARGS__)

#define LogWin32Info(msg, ...) Ether::Log(Ether::LogLevel::LOGLEVEL_INFO, Ether::LogType::LOGTYPE_WIN32, msg, ##__VA_ARGS__)
#define LogWin32Warning(msg, ...) Ether::Log(Ether::LogLevel::LOGLEVEL_WARNING, Ether::LogType::LOGTYPE_WIN32, msg, ##__VA_ARGS__)
#define LogWin32Error(msg, ...) Ether::Log(Ether::LogLevel::LOGLEVEL_ERROR, Ether::LogType::LOGTYPE_WIN32, msg, ##__VA_ARGS__)
#define LogWin32Fatal(msg, ...) Ether::Log(Ether::LogLevel::LOGLEVEL_FATAL, Ether::LogType::LOGTYPE_WIN32, msg, ##__VA_ARGS__)

ETH_ENGINE_DLL void Log(LogLevel level, LogType type, const char* fmt, ...);

class LoggingManager : public NonCopyable
{
public:
    void Initialize();
    void Shutdown();
    void AddLog(const LogEntry entry);
    void Serialize(const LogEntry entry);
    void Clear();

    inline const std::vector<LogEntry> GetEntries() const { return m_LogEntries; }

private:
    const std::wstring GetOutputDirectory() const;
    const std::wstring GetTimestampedFileName() const;

private:
    std::vector<LogEntry> m_LogEntries;
    std::wfstream m_LogFileStream;
};

ETH_NAMESPACE_END