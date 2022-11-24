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

namespace Ether
{
    enum class LogLevel
    {
        Info,
        Warning,
        Error,
        Fatal
    };

    enum class LogType
    {
        Engine,
        Graphics,
        Win32,
        Toolmode,
        None,
    };

    class LogEntry
    {
    public:
        LogEntry(const std::string& text, LogLevel level, LogType type);

        std::string GetText() const;
        std::string GetFullText() const;
        std::string GetLogLevelPrefix() const;
        std::string GetLogTypePrefix() const;
        std::string GetTimePrefix() const;

    private:
        std::string FormatTime(const time_t t, const char* format) const;

    public:
        const std::string m_Text;
        const LogLevel m_Level;
        const LogType m_Type;
        const uint64_t m_Time;
    };
}

