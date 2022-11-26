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

#ifdef ETH_PLATFORM_WIN32

#include "engine/platform/win32/win32launchargs.h"
#include <windows.h>
#include <string>
#include <sstream>
#include <processenv.h>

Ether::Win32::Win32LaunchArgs::Win32LaunchArgs()
{
    LPWSTR cmdLineRaw = GetCommandLineW();
    int argc = 0;
    LPWSTR* argv = CommandLineToArgvW(cmdLineRaw, &argc);

    for (int i = 1; i < argc; ++i) // Start from 1 to ignore application path
    {
        std::wstring flag = argv[i];
        std::wstring arg = L"";

        if (i + 1 < argc && argv[i + 1][0] != '-')
            arg = argv[++i];

        m_LaunchArgs.emplace_back(ToNarrowString(flag), ToNarrowString(arg));
    }
}

#endif // ETH_PLATFORM_WIN32

