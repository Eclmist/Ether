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

#include "api.h"
#include "sample/ethersample.h"

EtherSample* m_EtherEditorInstance;

LRESULT WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    // TODO: Setup proper API error handling, whether the api has been initialized or not etc.
    if (m_EtherEditorInstance == nullptr)
        return 0;

    // TODO: Fix this hack. WndProcInternal should not be directly called. (and should be private)
    m_EtherEditorInstance->GetWindowManager()->WndProcInternal(hWnd, msg, wParam, lParam);
    return 0;
}

void Initialize(HWND hWnd)
{
    EngineConfig engineConfig;
    engineConfig.SetIsRunningInEditor(true);
    engineConfig.SetEditorHwnd(hWnd);

    m_EtherEditorInstance = new EtherSample(engineConfig);
    m_EtherEditorInstance->Initialize();
}

void Update()
{
    // TODO: Figure out how to properly populate this args
    UpdateEventArgs e;
    m_EtherEditorInstance->OnUpdate(e);
}

void Release()
{
    m_EtherEditorInstance->Shutdown();
    delete m_EtherEditorInstance;
}

