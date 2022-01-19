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
    case LogLevel::Warning:
        return " WARN -";
    case LogLevel::Error:
        return "ERROR -";
    case LogLevel::Fatal:
        return "FATAL -";
    case LogLevel::Info:
    default:
        return " INFO -";
    }
}

std::string LogEntry::GetLogTypePrefix() const
{
    switch (m_Type)
    {
    case LogType::Engine:
        return "[  Engine  ]";
    case LogType::Graphics:
        return "[ Graphics ]";
    case LogType::Win32:
        return "[ Platform ]";
    case LogType::Toolmode:
        return "[ Toolmode ]";
    case LogType::None:
    default:
        return "[  Engine  ]";
    }
}

std::string LogEntry::GetTimePrefix() const
{
    return "[ " + FormatTime(m_Time, "%Y/%m/%d %X") + " ]";
}

ETH_NAMESPACE_END

