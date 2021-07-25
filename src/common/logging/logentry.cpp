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

#include "logentry.h"

ETH_NAMESPACE_BEGIN

LogEntry::LogEntry(const std::string& text, LogLevel level, LogType type)
    : m_Text(text)
    , m_Level(level)
    , m_Type(type)
    , m_Time(GetSystemTime())
{
}

std::string LogEntry::GetText() const
{
    return GetTimePrefix() + " " + GetLogTypePrefix() + " " + m_Text;
}

std::string LogEntry::GetFullText() const
{
    return GetTimePrefix() + " " + GetLogTypePrefix() + " " + GetLogLevelPrefix() + " " + m_Text;
}

std::string LogEntry::GetLogLevelPrefix() const
{
    switch (m_Level)
    {
    case LogLevel::LOGLEVEL_WARNING:
        return " WARN -";
    case LogLevel::LOGLEVEL_ERROR:
        return "ERROR -";
    case LogLevel::LOGLEVEL_FATAL:
        return "FATAL -";
    case LogLevel::LOGLEVEL_INFO:
    default:
        return " INFO -";
    }
}

std::string LogEntry::GetLogTypePrefix() const
{
    switch (m_Type)
    {
    case LogType::LOGTYPE_ENGINE:
        return "[  Engine  ]";
    case LogType::LOGTYPE_GRAPHICS:
        return "[ Graphics ]";
    case LogType::LOGTYPE_WIN32:
        return "[ Platform ]";
    case LogType::LOGTYPE_TOOLMODE:
        return "[ Toolmode ]";
    case LogType::LOGTYPE_NONE:
    default:
        return "[  Engine  ]";
    }
}

std::string LogEntry::GetTimePrefix() const
{
    return "[ " + FormatTime(m_Time) + " ]";
}

ETH_NAMESPACE_END
