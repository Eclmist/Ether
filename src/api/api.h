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

#pragma once

#include "api/iapplicationbase.h"

#include "common/time/time.h"
#include "engine/input/input.h"
#include "engine/world/world.h"
#include "common/logging/loggingmanager.h"
#include "engine/config/commandlineoptions.h"
#include "graphics/config/graphicconfig.h"

namespace Ether
{
ETH_ENGINE_DLL int Start(IApplicationBase& app);
ETH_ENGINE_DLL void Shutdown();

ETH_ENGINE_DLL CommandLineOptions& GetCommandLineOptions();
ETH_ENGINE_DLL World& GetActiveWorld();
} // namespace Ether

namespace Ether::Graphics
{
ETH_ENGINE_DLL GraphicConfig& GetGraphicConfig();
}

namespace Ether::Client
{
ETH_ENGINE_DLL void SetClientTitle(const std::string& title);
ETH_ENGINE_DLL std::string GetClientTitle();

ETH_ENGINE_DLL void SetClientSize(const ethVector2u& size);
ETH_ENGINE_DLL ethVector2u GetClientSize();

ETH_ENGINE_DLL void SetFullscreen(bool enabled);
ETH_ENGINE_DLL bool IsFullscreen();
} // namespace Ether::Client

namespace Ether::Toolmode
{
ETH_ENGINE_DLL void SetParentWindow(void* hwnd);
ETH_ENGINE_DLL void* GetWindowHandle();

ETH_ENGINE_DLL void ShowWindow();
ETH_ENGINE_DLL void HideWindow();
} // namespace Ether::Toolmode
