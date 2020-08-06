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

class EtherSample : public Engine
{
public:
    EtherSample(const EngineConfig& config);

    void Initialize() override;

    void LoadContent() override;

    void UnloadContent() override;

    void Shutdown() override;

    void OnUpdate(UpdateEventArgs& e) override;

    void OnRender(RenderEventArgs& e) override;

    void OnKeyPressed(KeyEventArgs& e) override;

    void OnKeyReleased(KeyEventArgs& e) override;

    void OnMouseButtonPressed(MouseEventArgs& e) override;

    void OnMouseButtonReleased(MouseEventArgs& e) override;

    void OnMouseMoved(MouseEventArgs& e) override;
};
