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

#include "enginesubsystemscheduler.h"
#include "engine/enginesubsystemregistry.h"

void EngineSubsystemScheduler::InitializeSubsystems()
{
    //ImGuiManager::GetInstance().Initialize();
    //WindowManager::GetInstance().Initialize();
    //GfxRenderer::GetInstance().Initialize();

    EngineSubsystemRegistry subsystemRegistry = EngineSubsystemRegistry::GetInstance();
}

void EngineSubsystemScheduler::ShutdownSubsystems()
{
    //ImGuiManager::GetInstance().Shutdown();
    //WindowManager::GetInstance().Shutdown();
    //GfxRenderer::GetInstance().Shutdown();
}
