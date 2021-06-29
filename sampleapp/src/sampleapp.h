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

#include "core/core.h"
#include "graphic/graphiccore.h"
#include "graphic/resource/visual.h"

class SampleApp : public Ether::ApplicationBase
{
public:
    void Initialize() override;
    void LoadContent() override;
    void UnloadContent() override;
    void Shutdown() override;

public:
    void OnUpdate(const Ether::UpdateEventArgs& e) override;
    void OnRender(const Ether::RenderEventArgs& e) override;
    void OnKeyPress(const Ether::KeyEventArgs& e) override;
    void OnKeyRelease(const Ether::KeyEventArgs& e) override;
    void OnMouseButtonPress(const Ether::MouseEventArgs& e) override;
    void OnMouseButtonRelease(const Ether::MouseEventArgs& e) override;
    void OnMouseMove(const Ether::MouseEventArgs& e) override;

private:
    Ether::Visual* m_DebugCube;
};
