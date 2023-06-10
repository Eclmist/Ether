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

#include "ether.h"
#include <string>
#include "engine/world/ecs/components/ecstransformcomponent.h"

class SampleApp : public Ether::IApplicationBase
{
public:
    void Initialize() override;
    void LoadContent() override;
    void UnloadContent() override;
    void Shutdown() override;

public:
    void OnUpdate(const Ether::UpdateEventArgs& e) override;
    void OnRender(const Ether::RenderEventArgs& e) override;
    void OnShutdown() override;

private:
    Ether::Ecs::EcsTransformComponent* m_CameraTransform;
};
