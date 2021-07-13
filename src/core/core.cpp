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

ETH_NAMESPACE_BEGIN

void EngineCore::Initialize(IApplicationBase& app)
{
    Instance().m_MainApplication = &app;
    Instance().m_MainWindow = std::make_unique<Window>();
    Instance().m_ActiveWorld = std::make_unique<World>();
}

void EngineCore::LoadContent()
{
    // Other engine content can be loaded here before or after main application
    Instance().m_MainApplication->LoadContent();
}

void EngineCore::Shutdown()
{
    Instance().m_ActiveWorld.reset();
    Instance().m_MainWindow.reset();
    Reset();
}

ETH_NAMESPACE_END

