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
#include <stdio.h>
#include <shellapi.h>
#include <stdlib.h>

Ether::Win32::Win32LaunchArgs::Win32LaunchArgs()
{
    // TODO: Get windows command line args 
    m_LaunchArgs.emplace_back("-sourceshaders", "");
    m_LaunchArgs.emplace_back("-usevalidationlayer", "");
    m_LaunchArgs.emplace_back("-toolmodeport", "2134");
}

#endif // ETH_PLATFORM_WIN32

