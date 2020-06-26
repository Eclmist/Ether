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

#include "subsystemscheduler.h"
#include "subsystem.h"

#include "imgui/imguimanager.h"
#include "win32/windowmanager.h"
#include "graphic/gfxrenderer.h"

void SubSystemScheduler::DeclareDependency(const USSID& dependency)
{

}

void SubSystemScheduler::InitializeSubSystems()
{
    // TODO: Make proper scheduler
    ImGuiManager::InitSingleton();
    WindowManager::InitSingleton(L"Ether");
    GfxRenderer::InitSingleton();
}


void SubSystemScheduler::ShutdownSubSystems()
{
    // TODO: Make proper scheduler
    GfxRenderer::DestroySingleton();
    WindowManager::DestroySingleton();
    ImGuiManager::DestroySingleton();
}

void SubSystemScheduler::BuildDependencyGraph()
{
    // Basically just topological sort, nothing special.
    BuildAdjacencyList();


    //std::stack<SubSystem*> stack;

    //const uint32_t numSubSystems;

    //bool visited[] = new bool[numSubSystems];
    //for (int i = 0; i < numSubSystems; ++i)
    //    visited[i] = false;



}

void SubSystemScheduler::BuildAdjacencyList()
{

}
