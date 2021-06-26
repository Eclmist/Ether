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

#include "logger.h"
#include <iosfwd>
#include <fstream>
#include <ShlObj_core.h>

ETH_NAMESPACE_BEGIN

void Log(LogLevel level, LogType type, const char* fmt, ...)
{
    char formattedBuffer[4096];

    va_list args;
    va_start(args, fmt);
    vsprintf_s(formattedBuffer, fmt, args);
    va_end(args);

    std::string formattedText(formattedBuffer);
    Logger::GetInstance().AddLog(LogEntry(formattedText, level, type));
}


Logger::Logger()
{
    Clear();
}

void Logger::AddLog(const LogEntry&& entry)
{
    m_LogEntries.push_back(entry);
}

void Logger::Clear()
{
    m_LogEntries.clear();
}

void Logger::Serialize()
{
    std::wfstream outfile;
    outfile.open(std::wstring(GetOutputDirectory() + L"/" + GetTimestampedFileName()), std::ios_base::app);

    if (!outfile.is_open()) 
    {
        LogInfo("Unable to open log file for serialization. Logs may be lost.\n");
        return;
    }

    for (int i = 0; i < m_LogEntries.size(); ++i)
        outfile << m_LogEntries[i].GetText().c_str() << '\n';

    outfile.close();
}

const std::wstring Logger::GetOutputDirectory() const
{
    wchar_t path[1024];
    wchar_t* appDataLocal;
    SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &appDataLocal);
    wcscpy_s(path, appDataLocal);
    wcscat_s(path, L"\\Ether");
    CreateDirectory(path, NULL);
    wcscat_s(path, L"\\");
    wcscat_s(path, g_MainApplication->GetClientName());
    CreateDirectory(path, NULL);
    wcscat_s(path, L"\\Logs");
    CreateDirectory(path, NULL);
    return path;
}

const std::wstring Logger::GetTimestampedFileName() const
{
    wchar_t filename[1024];
    wcscpy_s(filename, WFormatTime(GetSystemTime(), L"%Y%m%d%H%M%S").c_str()); // TODO: Replace with start time
    wcscat_s(filename, L".log");
    return filename;
}

ETH_NAMESPACE_END
