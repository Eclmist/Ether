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

#pragma once

#include "common/common.h"
#include "common/logging/logentry.h"
#include <fstream>
#include <deque>
#include <mutex>

namespace Ether
{
    class ETH_COMMON_DLL LoggingManager : public Singleton<LoggingManager>
    {
    public:
        LoggingManager() = default;
        ~LoggingManager();

		void Initialize();

    public:
        void Log(LogLevel level, LogType type, const char* fmt, ...);

    private:
        void AddLog(const LogEntry entry);
        void Serialize(const LogEntry entry);
        void Clear();

        inline const std::deque<LogEntry>& GetEntries() const { return m_LogEntries; }

    private:
        const std::string GetOutputDirectory() const;
        const std::string GetTimestampedFileName() const;
    private:
        std::deque<LogEntry> m_LogEntries;
        std::ofstream m_LogFileStream;
        std::mutex m_Mutex;
    };

}

