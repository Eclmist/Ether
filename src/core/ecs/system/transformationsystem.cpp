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

#include "transformationsystem.h"

ETH_NAMESPACE_BEGIN

TransformationSystem::TransformationSystem()
{
    m_Signature.set(EngineCore::GetEcsManager().GetComponentID<TransformComponent>());
}

void TransformationSystem::OnEntityRegister(EntityID id)
{
}

void TransformationSystem::OnEntityDeregister(EntityID id)
{
}

void TransformationSystem::OnSceneLoad()
{
}

void TransformationSystem::OnUpdate()
{
    ETH_MARKER_EVENT("ECS - Transformation System - Update");

    for (EntityID id : m_MatchingEntities)
    {
        ETH_MARKER_EVENT("ECS - Transformation System - Update matrices");
        auto* transform = EngineCore::GetEcsManager().GetComponent<TransformComponent>(id);
        transform->UpdateMatrices();
    }
}

ETH_NAMESPACE_END

