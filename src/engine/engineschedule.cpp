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

#include "engineschedule.h"
#include "engine/engine.h"
#include "graphic/gfxrenderer.h"
#include "imgui/imguimanager.h"
#include "win32/windowmanager.h"

DECLARE_SUBSYSTEM(GfxRenderer);
DECLARE_SUBSYSTEM(ImGuiManager);
DECLARE_SUBSYSTEM(WindowManager);

EngineSchedule::EngineSchedule()
{
}

EngineSchedule::~EngineSchedule()
{
}

void EngineSchedule::ScheduleSubSystems()
{
    // Note: This list does not control execution order of subsystems.
    // Execution order will be determined based on dependencies declared in 
    // SubSystem::RegisterDependencies. 

    RegisterSubsystem(USSID(GfxRenderer));
    RegisterSubsystem(USSID(ImGuiManager));
    RegisterSubsystem(USSID(WindowManager));
}

void EngineSchedule::RegisterSubsystem(const USSID& uid)
{
    Engine::GetInstance().RegisterSubsystem(uid);
}
