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

#include "toolmode/pch.h"
#include "engine/world/ecs/components/ecstransformcomponent.h"

namespace Ether::Toolmode
{
    class EtherHeadless : public Ether::IApplicationBase
    {
    public:
        void Initialize() override;
        void LoadContent() override;
        void UnloadContent() override;
        void Shutdown() override;

    public:
        void OnUpdate(const Ether::UpdateEventArgs& e) override;
        void OnRender(const Ether::RenderEventArgs& e) override;
        void OnPostRender() override;
        void OnShutdown() override;

    private:
        void UpdateGraphicConfig() const;
        void UpdateCamera() const;

    private:
        Ether::Ecs::EcsTransformComponent* m_CameraTransform;
    };
}

