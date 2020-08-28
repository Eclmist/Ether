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

#include "system/system.h"
#include "engine/config/engineconfig.h"
#include "engine/events/events.h"
#include "engine/subsystem/enginesubsystemcontroller.h"
#include "engine/subsystem/subsystem.h"

class Window;
class GfxRenderer;

class Engine : public NonCopyable
{
public:
    Engine(const EngineConfig& config);

    //! @brief Initialize the Ether engine.
    virtual void Initialize();

    //! @brief Load all content required.
    virtual void LoadContent() = 0;

    //! @brief Unload content that was loaded by LoadContent.
    virtual void UnloadContent() = 0;

    //! @brief Unload all content and shutdown engine.
    virtual void Shutdown();

    virtual void OnUpdate(UpdateEventArgs& e);

    virtual void OnRender(RenderEventArgs& e);

    virtual void OnKeyPressed(KeyEventArgs& e);

    virtual void OnKeyReleased(KeyEventArgs& e);

    virtual void OnMouseButtonPressed(MouseEventArgs& e);

    virtual void OnMouseButtonReleased(MouseEventArgs& e);

    virtual void OnMouseMoved(MouseEventArgs& e);

public:
    inline const EngineConfig GetEngineConfig() const { return m_EngineConfig; };

    inline Window* GetWindow() const { return m_SubsystemController.GetWindow(); };
    inline GfxRenderer* GetRenderer() const { return m_SubsystemController.GetRenderer(); };

protected:
    EngineConfig m_EngineConfig;
    EngineSubsystemController m_SubsystemController;
};

