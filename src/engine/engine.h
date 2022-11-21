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

#pragma once

#include "api/iapplicationbase.h"
#include "engine/config/engineconfig.h"
#include "engine/config/commandlineoptions.h"
#include "engine/platform/platformwindow.h"

#include "graphics/core.h"

namespace Ether
{
    class Engine : public Singleton<Engine>
    {
    public:
        Engine() = default;
        ~Engine() = default;

		void Initialize();
        void Shutdown();

    public:
        static EngineConfig& GetEngineConfig() { return Instance().m_EngineConfig; }
        static CommandLineOptions& GetCommandLineOptions() { return Instance().m_CommandLineOptions; }
        static PlatformWindow& GetMainWindow() { return *Instance().m_MainWindow; }

        static bool IsInitialized() { return Instance().m_IsInitialized; }

    public:
        void LoadApplication(IApplicationBase& app);

	public:
		void Run();

	private:
        void MainEngineThread();

    private:
        std::atomic_bool m_ShouldExit;
        std::unique_ptr<PlatformWindow> m_MainWindow;

        IApplicationBase* m_MainApplication;
        EngineConfig m_EngineConfig;
        CommandLineOptions m_CommandLineOptions;

        bool m_IsInitialized;
    };
}

