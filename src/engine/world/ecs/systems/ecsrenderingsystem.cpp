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

#include "engine/enginecore.h"
#include "engine/world/entity.h"
#include "engine/world/ecs/systems/ecsrenderingsystem.h"
#include "engine/world/ecs/components/ecsvisualcomponent.h"

Ether::Ecs::EcsRenderingSystem::EcsRenderingSystem(const EcsComponentManager& componentMgr)
    : EcsSystem(componentMgr)
{
    m_Signature.set(EcsVisualComponent::s_ComponentID);
}

void Ether::Ecs::EcsRenderingSystem::Update()
{
    ETH_MARKER_EVENT("Rendering System - Update");

    for (EntityID entityID : m_Entities)
    {
        Entity& entity = EngineCore::GetActiveWorld().GetEntity(entityID);
        EcsVisualComponent& visual = entity.GetComponent<EcsVisualComponent>();

        if (!visual.m_Enabled)
            continue;

        LogInfo("%s", entity.GetName().c_str());
    }
}

