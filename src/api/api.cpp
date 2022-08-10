/*
    This file is part of Ether, an open-source DirectX 12 renderer.

    Copyright (c) 2020-2022 Samuel Huang - All rights reserved.

    Ether is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "api.h"
#include "graphic/graphic.h"
#include <dxgidebug.h>

ETH_NAMESPACE_BEGIN

void UpdateEngine()
{
    while (true)
    {
		OPTICK_FRAME("Engine - MainThread");

		static double lastTime = 0;

		{
			OPTICK_FRAME("Application (ToolMain) - Update");
			UpdateEventArgs updateArgs;
			updateArgs.m_TotalElapsedTime = GetTimeSinceStart();
			updateArgs.m_DeltaTime = updateArgs.m_TotalElapsedTime - lastTime;

			EngineCore::GetMainApplication().OnUpdate(updateArgs);
		}

		EngineCore::Update();

		RenderEventArgs renderArgs;
		renderArgs.m_TotalElapsedTime = GetTimeSinceStart();
		renderArgs.m_GraphicContext = &GraphicCore::GetGraphicRenderer().GetGraphicContext();
		renderArgs.m_DeltaTime = renderArgs.m_TotalElapsedTime - lastTime;
		lastTime = renderArgs.m_TotalElapsedTime;

		{
			OPTICK_EVENT("Application - OnRender");
			EngineCore::GetMainApplication().OnRender(renderArgs);
		}
		{
			OPTICK_EVENT("Graphic Core - Render");
			GraphicCore::Render();
		}

		ETH_TOOLONLY(EngineCore::GetIpcManager().ProcessPendingCommands());

        if (EngineCore::GetMainApplication().ShouldExit())
            break;
    }
}

void WindowsUpdateLoop()
{
    while(true)
    {
        Input::Instance().NewFrame(); // Todo: make it Input::Newframe();

        MSG msg = {};
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        if (msg.message == WM_QUIT)
            break;
    } 
}

int Start(IApplicationBase& app)
{
    LogInfo("Starting Ether v%d.%d.%d", 0, 1, 0);

    EngineCore::Initialize(app);
    GraphicCore::Initialize();

    EngineCore::LoadContent();
    ETH_ENGINEONLY(EngineCore::GetMainWindow().Show());

	std::thread mainEngineThread(UpdateEngine);
    WindowsUpdateLoop();

    mainEngineThread.join();

    GraphicCore::Shutdown();
    EngineCore::Shutdown();

#ifdef _DEBUG
    wrl::ComPtr<IDXGIDebug1> dxgiDebug;
    DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiDebug));
    dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_FLAGS(DXGI_DEBUG_RLO_SUMMARY | DXGI_DEBUG_RLO_IGNORE_INTERNAL));
#endif

    return 0;
}

ETH_NAMESPACE_END

