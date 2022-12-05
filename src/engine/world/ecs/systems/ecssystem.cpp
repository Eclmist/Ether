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

#include "engine/world/ecs/systems/ecssystem.h"
#include "engine/world/ecs/components/ecsmetadatacomponent.h"
#include "engine/world/ecs/components/ecstransformcomponent.h"

Ether::Ecs::EcsSystem::EcsSystem()
{
    m_Signature.set(EcsMetadataComponent::s_ComponentID);
    m_Signature.set(EcsTransformComponent::s_ComponentID);
}

Ether::Ecs::EcsSystem::~EcsSystem() = default;

