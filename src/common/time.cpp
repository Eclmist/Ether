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

time_t GetSystemTime()
{
    return time(0);
}

std::string FormatTime(const time_t t, char* format)
{
    char formatted[80];
    tm newTime;
    localtime_s(&newTime, &t);
    strftime(formatted, 80, format, &newTime);
    return formatted;
}

std::wstring WFormatTime(const time_t t, wchar_t* format)
{
    wchar_t formatted[80];
    tm newTime;
    localtime_s(&newTime, &t);
    wcsftime(formatted, 80, format, &newTime);
    return formatted;
}

ETH_NAMESPACE_END

