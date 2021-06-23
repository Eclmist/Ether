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
#include <imgui/imgui.h>

class Logger : public Singleton<Logger>
{
public:
    enum LogLevel
    {
        LOGLEVEL_INFO,
        LOGLEVEL_WARNING,
        LOGLEVEL_ERROR,
        LOGLEVEL_FATAL
    };

    enum LogType
    {
        LOGTYPE_ENGINE,
        LOGTYPE_GRAPHICS,
        LOGTYPE_WIN32,
        LOGTYPE_NONE,
    };

    Logger();
    ~Logger() = default;

public:
    static void Log(LogLevel level, LogType type, const char* fmt, ...);
    void Clear();
    void DrawImGui();
    void Serialize();

private:
    void AppendLogLevelPrefix(LogLevel level);
    void AppendLogTypePrefix(LogType type);
    const std::wstring GetOutputDirectory() const;
    const std::wstring GetTimestampedFileName() const;

private:
    ImGuiTextBuffer m_Buffer;
    std::vector<int> m_LineOffsets;
};

