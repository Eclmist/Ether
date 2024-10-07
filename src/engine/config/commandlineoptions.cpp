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
#include "engine/platform/win32/win32launchargs.h"

#if defined(ETH_TOOLMODE)
constexpr uint16_t DefaultToolmodePort = 2134;
#endif

Ether::CommandLineOptions::CommandLineOptions()
    : m_UseSourceShaders(false)
    , m_UseShaderDaemon(false)
    , m_UseValidationLayer(false)
    , m_WorldName("")
    , m_ShaderSourcePath(".\\Data\\shaders\\")
    , m_RTCampMode(false)
    , m_RTCampDebugMode(false)
#if defined(ETH_TOOLMODE)
    , m_WorkspacePath("")
    , m_ToolmodePort(2134)
    , m_ImportScale(1.0f)
    , m_ImportPaths()
#endif
{
    std::unique_ptr<PlatformLaunchArgs> args;
    args = std::make_unique<Win32::Win32LaunchArgs>();

    auto launchArgs = args->GetLaunchArgs();

    for (auto pair : launchArgs)
        RegisterSingleOption(pair.first, pair.second);
}

void Ether::CommandLineOptions::RegisterSingleOption(const std::string& flag, const std::string& arg)
{
    bool isValid = true;

    if (flag == "-sourceshaders")
        { m_UseSourceShaders = true; m_ShaderSourcePath = arg; }
    else if (flag == "-shaderdaemon")
        m_UseShaderDaemon = true;
    else if (flag == "-validationlayer")
        m_UseValidationLayer = true;
    else if (flag == "-world")
        m_WorldName = arg;
    else if (flag == "-rtcamp")
        m_RTCampMode = true;
    else if (flag == "-rtcampdebug")
        m_RTCampDebugMode = true;
#if defined(ETH_TOOLMODE)
    else if (flag == "-workspace")
        m_WorkspacePath = arg;
    else if (flag == "-importscale")
        m_ImportScale = stof(arg);
    else if (flag == "-import")
        m_ImportPaths.push_back(arg);
    else if (flag == "-toolmodeport")
        m_ToolmodePort = stoi(arg);
#endif
    else
    {
        LogWarning("Invalid command line argument found: %s", flag.c_str());
        isValid = false;
    }

    if (isValid)
        LogEngineInfo("Registered Command Line Argument: %s %s", flag.c_str(), arg.c_str());

}
