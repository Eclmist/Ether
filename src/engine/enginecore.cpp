/*
    This file is part of Ether, an open-source DirectX 12 renderer.

    Copyright (c) 2020-2030 Samuel Huang - All rights reserved.

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

#include "engine/enginecore.h"
#include "engine/platform/win32/win32window.h"
#include "engine/platform/win32/win32notificationtray.h"

void Ether::EngineCore::Initialize()
{
    m_MainWindow = std::make_unique<Win32::Win32Window>();
    m_NotificationTray = std::make_unique<Win32::Win32NotificationTray>();
    m_ActiveWorld = std::make_unique<World>();


#if ETH_TOOLMODE
    if (m_CommandLineOptions.ShouldInitializeRenderer())
#endif
    {
        InitializeGraphicsLayer();
    }

    m_IsInitialized = true;
}

void Ether::EngineCore::LoadApplication(IApplicationBase& app)
{
    m_MainApplication = &app;
    m_MainApplication->Initialize();
    m_MainApplication->LoadContent();

    // Create a default camera in case there isn't any. Need to separate scene camear from editor camera in the future (TODO)
    m_ActiveWorld->CreateCamera();
}

void Ether::EngineCore::MainEngineLoop()
{
    while (!m_ShutdownQueued)
    {
        ETH_MARKER_FRAME("Engine Thread");

        Time::NewFrame();
        Input::NewFrame();

        if (!m_MainWindow->ProcessPlatformMessages())
            break;

        m_MainApplication->OnUpdate({});
        m_ActiveWorld->Update();

        Graphics::GraphicCore::WaitForLastFrame();

        m_MainApplication->OnPreRender({});
        Graphics::GraphicCore::GetGraphicConfig().SetResolution(m_EngineConfig.GetClientSize());
        Graphics::GraphicCore::NewFrame();
        m_MainApplication->OnPostRender();
    }

    Graphics::GraphicCore::FlushGpu();

    m_MainApplication->OnShutdown();

    m_ActiveWorld.reset();
    m_NotificationTray.reset();
    m_MainWindow.reset();

    m_IsInitialized = false;

    Graphics::GraphicCore::Instance().Shutdown();
}

void Ether::EngineCore::Shutdown()
{
    m_ShutdownQueued = true;
}

void Ether::EngineCore::InitializeGraphicsLayer()
{
    Graphics::GraphicConfig& config = Graphics::GraphicCore::GetGraphicConfig();
    config.SetWindowHandle(m_MainWindow->GetWindowHandle());
    config.SetValidationLayerEnabled(m_CommandLineOptions.UseValidationLayer());
    config.SetUseShaderCache(m_CommandLineOptions.UseShaderCache());
    config.SetUseShaderDaemon(m_CommandLineOptions.UseShaderDaemon());
    config.SetUseGraphicsThread(m_CommandLineOptions.UseGraphicsThread());
    config.SetShaderSourceDir(m_CommandLineOptions.GetShaderSourcePath());
    config.SetResolution(m_EngineConfig.GetClientSize());

    Graphics::GraphicCore::Instance().Initialize();
}

