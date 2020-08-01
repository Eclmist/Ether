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

#include "engine/engine.h"
#include "engine/config/engineconfig.h"
#include "engine/events/events.h"

class Application : public NonCopyable
{
public:
    Application(const EngineConfig& config);

    //! @brief Initialize the Ether engine.
    virtual void Initialize();

    //! @brief Load all content required.
    virtual void LoadContent() = 0;

    //! @brief Unload content that was loaded by LoadContent.
    virtual void UnloadContent() = 0;

    //! @brief Destroy all resources that are being used.
    virtual void Destroy();

public:
    inline Engine* GetEngine() const { return m_Engine.get(); };

protected:
    virtual void OnUpdate(UpdateEventArgs& e);

    virtual void OnRender(RenderEventArgs& e);

    virtual void OnKeyPressed(KeyEventArgs& e);

    virtual void OnKeyReleased(KeyEventArgs& e);

    virtual void OnMouseButtonPressed(MouseEventArgs& e);

    virtual void OnMouseButtonReleased(MouseEventArgs& e);

    virtual void OnMouseMoved(MouseEventArgs& e);

protected:
    std::unique_ptr<Engine> m_Engine;
};
