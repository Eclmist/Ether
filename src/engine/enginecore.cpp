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

#include "engine/enginecore.h"
#include "engine/platform/win32/win32window.h"

void Ether::EngineCore::Initialize()
{
    m_MainWindow = std::make_unique<Win32::Win32Window>();

    // Deserialize world here?
    m_ActiveWorld = std::make_unique<World>();

    InitializeGraphicsLayer();

    m_IsInitialized = true;
}

void Ether::EngineCore::LoadApplication(IApplicationBase& app)
{
    m_MainApplication = &app;
    m_MainApplication->Initialize();
    m_MainApplication->LoadContent();
}

void Ether::EngineCore::Run()
{
    m_MainEngineThread = std::thread(&EngineCore::MainEngineThread, this);
    m_MainWindow->PlatformMessageLoop();
    m_MainEngineThread.join();
}

void Ether::EngineCore::Shutdown()
{
    m_MainApplication->OnShutdown();

    // Release GPU mesh resources before shutting down graphics layer
    Graphics::GraphicCore::FlushGpu();
    m_ActiveWorld.reset();

    Graphics::GraphicCore::Instance().Shutdown();
}

void Ether::EngineCore::InitializeGraphicsLayer()
{
    Graphics::GraphicConfig& config = Graphics::GraphicCore::GetGraphicConfig();
    config.SetWindowHandle(m_MainWindow->GetWindowHandle());
    config.SetValidationLayerEnabled(m_CommandLineOptions.GetUseValidationLayer());
    config.SetUseSourceShaders(m_CommandLineOptions.GetUseSourceShaders());
    config.SetUseShaderDaemon(m_CommandLineOptions.GetUseShaderDaemon());
    config.SetShaderSourceDir(
        m_CommandLineOptions.GetUseSourceShaders() ? "../../../src/graphics/shaders/" : "Data/shaders/");

    Graphics::GraphicCore::Instance().Initialize();
}

void Ether::EngineCore::MainEngineThread()
{
    while (true)
    {
        ETH_MARKER_FRAME("Engine Frame");

        Time::NewFrame();
        Input::NewFrame();

        if (!m_MainWindow->ProcessPlatformMessages())
            break;

        m_MainApplication->OnUpdate({});

        m_ActiveWorld->Update();
        Graphics::GraphicCore::Main();

        Input::EndFrame();
    }
}
