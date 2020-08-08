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

HWND Initialize(HWND host)
{
    EngineConfig engineConfig;
    engineConfig.SetIsRunningInEditor(true);
    engineConfig.SetEditorHwndHost(host);

    m_EtherEditorInstance = new EtherSample(engineConfig);
    m_EtherEditorInstance->Initialize();

    m_EtherEditorInstance->GetWindow()->Show();

    return m_EtherEditorInstance->GetWindow()->GetHwnd();
}

void ResizeViewport(uint32_t width, uint32_t height)
{
    m_EtherEditorInstance->OnEditorViewportResize(width, height);
}

void Update()
{
    // TODO: Figure out how to properly populate this args
    UpdateEventArgs e;
    m_EtherEditorInstance->OnUpdate(e);
}

ETH_EXPORT_DLL void Render()
{
    // TODO: Figure out how to properly populate this args
    RenderEventArgs e;
    m_EtherEditorInstance->OnRender(e);
}

void Release()
{
    m_EtherEditorInstance->Shutdown();
    delete m_EtherEditorInstance;
}

