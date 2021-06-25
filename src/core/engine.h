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

#include "core/config/engineconfig.h"
#include "core/event/events.h"

ETH_NAMESPACE_BEGIN

class Engine : public Singleton<Engine>
{
public:
    Engine();

    void Initialize(const EngineConfig& config);

    void LoadContent();

    void UnloadContent();

    void Shutdown();

    void OnUpdate(UpdateEventArgs& e);

    void OnRender(RenderEventArgs& e);

    void OnKeyPressed(KeyEventArgs& e);

    void OnKeyReleased(KeyEventArgs& e);

    void OnMouseButtonPressed(MouseEventArgs& e);

    void OnMouseButtonReleased(MouseEventArgs& e);

    void OnMouseMoved(MouseEventArgs& e);

public:
    inline const EngineConfig GetEngineConfig() const { return m_EngineConfig; };

private:
    void InitializeSubsystems();
    void ShutdownSubsystems();

private:
    EngineConfig m_EngineConfig;
};

ETH_NAMESPACE_END
