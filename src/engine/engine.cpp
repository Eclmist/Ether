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

#include "engine/engine.h"

#if defined(ETH_PLATFORM_WIN32)
#include "engine/platform/win32/win32window.h"
#elif defined(ETH_PLATFORM_PS5)
#include "engine/platform/ps5/ps5window.h"
#endif

void Ether::Engine::Initialize()
{
#if defined(ETH_PLATFORM_WIN32)
    m_MainWindow = std::make_unique<Win32::Win32Window>();
#elif defined(ETH_PLATFORM_PS5)
    m_MainWindow = std::make_unique<PS5::PS5Window>();
#endif

    InitializeGraphicsLayer();

    m_IsInitialized = true;
}

void Ether::Engine::LoadApplication(IApplicationBase& app)
{
    m_MainApplication = &app;
    m_MainApplication->Initialize();
    m_MainApplication->LoadContent();
}

void Ether::Engine::Run()
{
    m_MainEngineThread = std::thread(&Engine::MainEngineThread, this);
    m_MainWindow->PlatformMessageLoop();
    m_MainEngineThread.join();
}

void Ether::Engine::Shutdown()
{
    m_MainApplication->OnShutdown();
    Graphics::Core::Reset();
    m_IsInitialized = false;
}

void Ether::Engine::InitializeGraphicsLayer()
{
    Graphics::GraphicConfig& config = Graphics::Core::Instance().GetGraphicConfig();
    config.SetWindowHandle(m_MainWindow->GetWindowHandle());
    config.SetValidationLayerEnabled(m_CommandLineOptions.GetUseValidationLayer());
    config.SetUseSourceShaders(m_CommandLineOptions.GetUseSourceShaders());
    config.SetShaderSourceDir(m_CommandLineOptions.GetUseSourceShaders() ? "../../../src/graphics/shaders/" : "Data/shaders/");

    Graphics::Core::Instance().Initialize();
}

void Ether::Engine::MainEngineThread()
{
    while (true)
    {
        ETH_MARKER_FRAME("Update");
        if (!m_MainWindow->ProcessPlatformMessages())
            break;

        Time::NewFrame();
        Input::NewFrame();

        {
            ETH_MARKER_EVENT("Game Application - OnUpdate");
            m_MainApplication->OnUpdate({}); // No useful args for now
        }

        Input::EndFrame();
        Graphics::Core::MainGraphicsThread();
    }
}

