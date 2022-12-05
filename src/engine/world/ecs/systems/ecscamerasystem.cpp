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
#include "engine/world/ecs/systems/ecscamerasystem.h"
#include "engine/world/ecs/components/ecscameracomponent.h"
#include "engine/world/ecs/components/ecstransformcomponent.h"

#include "graphics/graphiccore.h"

Ether::Ecs::EcsCameraSystem::EcsCameraSystem()
{
    m_Signature.set(EcsCameraComponent::s_ComponentID);
}

void Ether::Ecs::EcsCameraSystem::Update()
{
    ETH_MARKER_EVENT("Camera System - Update");

    Graphics::GraphicContext& gfxContext = Graphics::GraphicCore::GetGraphicRenderer().GetGraphicContext();

    for (EntityID entityID : m_Entities)
    {
        Entity& entity = EngineCore::GetActiveWorld().GetEntity(entityID);
        EcsCameraComponent& camera = entity.GetComponent<EcsCameraComponent>();
        EcsTransformComponent& transform = entity.GetComponent<EcsTransformComponent>();

        if (!camera.m_Enabled)
            continue;

        ethMatrix4x4 rotationInv = Transform::GetRotationMatrix(-transform.m_Rotation);
        ethMatrix4x4 translationInv = Transform::GetTranslationMatrix(-transform.m_Translation);
        ethMatrix4x4 viewMatrix = translationInv * rotationInv;
        gfxContext.SetViewMatrix(viewMatrix);

        ethVector2u resolution = EngineCore::GetEngineConfig().GetClientSize();
        float aspect = static_cast<float>(resolution.x) / resolution.y;
        gfxContext.SetProjectionMatrix(Transform::GetPerspectiveMatrixLH(SMath::DegToRad(camera.m_FieldOfView), aspect, 0.01f, 1000.0f));

        ethMatrix4x4 rotation = Transform::GetRotationMatrix(transform.m_Rotation);
        ethVector4 forward = rotation * ethVector4(0, 0, 1, 0);
        gfxContext.SetEyeDirection(forward.Resize<3>());
        gfxContext.SetEyePosition(transform.m_Translation);
    }
}

