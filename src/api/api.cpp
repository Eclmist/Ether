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
#include "api/interface/igameapplication.h"
#include "system/win32/window.h"

namespace EtherGame
{
    void InitializeApplication(iGameApplication& app)
    {
        // TODO: Init other engine systems here

        app.Initialize();
    }

    void TerminateApplication(iGameApplication& app)
    {
        // TODO: Terminate other engine systems here
        app.Shutdown();
    }

    bool UpdateApplication(iGameApplication& app)
    {
        app.Update();
        app.RenderScene();
        app.RenderGui();

        return !app.IsDone();
    }

    int Start(iGameApplication& app, const wchar_t* classname, HINSTANCE hInst, int cmdShow)
    {
        Window gameWindow(app, classname, hInst);
        InitializeApplication(app);
        gameWindow.Show(cmdShow);

        do
        {
            MSG msg = {};
            while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
            if (msg.message == WM_QUIT)
                break;
        } 
        while (UpdateApplication(app));

        TerminateApplication(app);
        return 0;
    }
}
