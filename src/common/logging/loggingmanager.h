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

#pragma once

#include "logentry.h"
#include <fstream>

ETH_NAMESPACE_BEGIN

#define LogInfo(msg, ...) Ether::Log(Ether::LogLevel::Info, Ether::LogType::None, msg, ##__VA_ARGS__)
#define LogWarning(msg, ...) Ether::Log(Ether::LogLevel::Warning, Ether::LogType::None, msg, ##__VA_ARGS__)
#define LogError(msg, ...) Ether::Log(Ether::LogLevel::Error, Ether::LogType::None, msg, ##__VA_ARGS__)
#define LogFatal(msg, ...) Ether::Log(Ether::LogLevel::Fatal, Ether::LogType::None, msg, ##__VA_ARGS__)

#define LogEngineInfo(msg, ...) Ether::Log(Ether::LogLevel::Info, Ether::LogType::Engine, msg, ##__VA_ARGS__)
#define LogEngineWarning(msg, ...) Ether::Log(Ether::LogLevel::Warning, Ether::LogType::Engine, msg, ##__VA_ARGS__)
#define LogEngineError(msg, ...) Ether::Log(Ether::LogLevel::Error, Ether::LogType::Engine, msg, ##__VA_ARGS__)
#define LogEngineFatal(msg, ...) Ether::Log(Ether::LogLevel::Fatal, Ether::LogType::Engine, msg, ##__VA_ARGS__)

#define LogGraphicsInfo(msg, ...) Ether::Log(Ether::LogLevel::Info, Ether::LogType::Graphics, msg, ##__VA_ARGS__)
#define LogGraphicsWarning(msg, ...) Ether::Log(Ether::LogLevel::Warning, Ether::LogType::Graphics, msg, ##__VA_ARGS__)
#define LogGraphicsError(msg, ...) Ether::Log(Ether::LogLevel::Error, Ether::LogType::Graphics, msg, ##__VA_ARGS__)
#define LogGraphicsFatal(msg, ...) Ether::Log(Ether::LogLevel::Fatal, Ether::LogType::Graphics, msg, ##__VA_ARGS__)

#define LogWin32Info(msg, ...) Ether::Log(Ether::LogLevel::Info, Ether::LogType::Win32, msg, ##__VA_ARGS__)
#define LogWin32Warning(msg, ...) Ether::Log(Ether::LogLevel::Warning, Ether::LogType::Win32, msg, ##__VA_ARGS__)
#define LogWin32Error(msg, ...) Ether::Log(Ether::LogLevel::Error, Ether::LogType::Win32, msg, ##__VA_ARGS__)
#define LogWin32Fatal(msg, ...) Ether::Log(Ether::LogLevel::Fatal, Ether::LogType::Win32, msg, ##__VA_ARGS__)

#ifdef ETH_TOOLMODE
#define LogToolmodeInfo(msg, ...) Ether::Log(Ether::LogLevel::Info, Ether::LogType::Toolmode, msg, ##__VA_ARGS__)
#define LogToolmodeWarning(msg, ...) Ether::Log(Ether::LogLevel::Warning, Ether::LogType::Toolmode, msg, ##__VA_ARGS__)
#define LogToolmodeError(msg, ...) Ether::Log(Ether::LogLevel::Error, Ether::LogType::Toolmode, msg, ##__VA_ARGS__)
#define LogToolmodeFatal(msg, ...) Ether::Log(Ether::LogLevel::Fatal, Ether::LogType::Toolmode, msg, ##__VA_ARGS__)
#endif

ETH_ENGINE_DLL void Log(LogLevel level, LogType type, const char* fmt, ...);

class LoggingManager : public NonCopyable
{
public:
    LoggingManager();
    ~LoggingManager();

    void AddLog(const LogEntry entry);
    void Serialize(const LogEntry entry);
    void Clear();

    inline const std::deque<LogEntry>& GetEntries() const { return m_LogEntries; }

private:
    const std::wstring GetOutputDirectory() const;
    const std::wstring GetTimestampedFileName() const;

private:
    std::deque<LogEntry> m_LogEntries;
    std::wfstream m_LogFileStream;
    std::mutex m_Mutex;
};

ETH_NAMESPACE_END