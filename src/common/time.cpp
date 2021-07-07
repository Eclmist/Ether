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

#include "time.h"

ETH_NAMESPACE_BEGIN

// TODO: Is this really the best way to make this utility thread safe?
std::mutex g_TimeMutex;

using namespace chrono;

time_point g_StartTime = steady_clock::now();

// TODO: Refactor time_t to use chrono
time_t GetSystemTime()
{
    std::lock_guard<std::mutex> guard(g_TimeMutex);
    return time(0);
}

std::string FormatTime(const time_t t, char* format)
{
    std::lock_guard<std::mutex> guard(g_TimeMutex);
    char formatted[80];
    tm newTime;
    localtime_s(&newTime, &t);
    strftime(formatted, 80, format, &newTime);
    return formatted;
}

std::wstring WFormatTime(const time_t t, wchar_t* format)
{
    std::lock_guard<std::mutex> guard(g_TimeMutex);
    wchar_t formatted[80];
    tm newTime;
    localtime_s(&newTime, &t);
    wcsftime(formatted, 80, format, &newTime);
    return formatted;
}

double GetStartTime()
{
    std::lock_guard<std::mutex> guard(g_TimeMutex);
    return duration_cast<milliseconds>(g_StartTime.time_since_epoch()).count() / 1000.0;
}

double GetTimeSinceStart()
{
    std::lock_guard<std::mutex> guard(g_TimeMutex);
    time_point now = chrono::steady_clock::now();
    return duration_cast<milliseconds>(now - g_StartTime).count() / 1000.0;
}

ETH_NAMESPACE_END

