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

ETH_NAMESPACE_BEGIN

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

public:
    const std::string m_Text;
    const LogLevel m_Level;
    const LogType m_Type;
    const time_t m_Time;
};

ETH_NAMESPACE_END