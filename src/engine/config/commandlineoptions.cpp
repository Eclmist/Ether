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

#include "engine/config/commandlineoptions.h"
#include "engine/platform/platformlaunchargs.h"

#if defined(ETH_PLATFORM_WIN32)
#include "engine/platform/win32/win32launchargs.h"
#elif defined(ETH_PLATFORM_PS5)
#include "engine/platform/ps5/ps5launchargs.h"
#endif

#if defined (ETH_TOOLMODE)
constexpr uint16_t DefaultToolmodePort = 2134;
#endif

Ether::CommandLineOptions::CommandLineOptions()
    : m_UseSourceShaders(false)
    , m_UseValidationLayer(false)
#if defined (ETH_TOOLMODE)
    , m_ToolmodePort(DefaultToolmodePort)
#endif
{
    std::unique_ptr<PlatformLaunchArgs> args;

#if defined(ETH_PLATFORM_WIN32)
    args = std::make_unique<Win32::Win32LaunchArgs>();
#elif defined(ETH_PLATFORM_PS5)
    args = std::make_unique<PS5::PS5LaunchArgs>();
#endif

    auto launchArgs = args->GetLaunchArgs();

    for (auto pair : launchArgs)
        RegisterSingleOption(pair.first, pair.second);
}

void Ether::CommandLineOptions::RegisterSingleOption(const std::string& flag, const std::string& arg)
{
    if (flag == "-sourceshaders")
        m_UseSourceShaders = true;
    else if (flag == "-usevalidationlayer")
        m_UseValidationLayer = true;
#if defined (ETH_TOOLMODE)
    else if (flag == "-toolmodeport")
        m_ToolmodePort = stoi(arg);
#endif
}

