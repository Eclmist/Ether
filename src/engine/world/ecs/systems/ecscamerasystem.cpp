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

    Graphics::GraphicConfig& gfxConfig = Graphics::GraphicCore::GetGraphicConfig();

    for (EntityID entityID : m_Entities)
    {
        Entity& entity = EngineCore::GetActiveWorld().GetEntity(entityID);
        EcsCameraComponent& camera = entity.GetComponent<EcsCameraComponent>();
        EcsTransformComponent& transform = entity.GetComponent<EcsTransformComponent>();

        // TODO: Jitter mode is set through imgui debug menu (which is in gfx project), so we need to update the component manually
        // In the future, this should be updated through the engine side, and this can be removed.
        camera.SetJitterMode((JitterMode)Graphics::GraphicCore::GetGraphicConfig().m_TemporalAAJitterMode);

        if (!camera.m_Enabled)
            continue;

        ethMatrix4x4 rotationInv = Transform::GetRotationMatrix(transform.m_Rotation).Inversed();
        ethMatrix4x4 translationInv = Transform::GetTranslationMatrix(-transform.m_Translation);
        ethMatrix4x4 viewMatrix = rotationInv * translationInv;

        ethVector2u resolution = EngineCore::GetEngineConfig().GetClientSize();
        float aspect = static_cast<float>(resolution.x) / resolution.y;

        ethMatrix4x4 projectionMatrix;
        switch (camera.m_ProjectionMode)
        {
        case ProjectionMode::Perspective:
            projectionMatrix = Transform::GetPerspectiveMatrixLH(
                SMath::DegToRad(camera.m_FieldOfView),
                aspect,
                camera.m_NearPlane,
                camera.m_FarPlane);
            break;
        }

        ethVector2 cameraJitter;
        if (gfxConfig.m_IsTemporalAAEnabled)
        {
            static uint32_t idx = 0;
            ethVector2 sample = camera.GetJitterOffset(idx++);
            cameraJitter.x = (sample.x - 0.5f) * gfxConfig.m_DebugJitterScale / resolution.x;
            cameraJitter.y = (sample.y - 0.5f) * gfxConfig.m_DebugJitterScale / resolution.y;

            projectionMatrix.m_13 += cameraJitter.x;
            projectionMatrix.m_23 += cameraJitter.y;
        }

        ethMatrix4x4 rotation = Transform::GetRotationMatrix(transform.m_Rotation);
        ethVector4 forward = rotation * ethVector4(0, 0, 1, 0);

        Graphics::RenderData& renderData = Graphics::GraphicCore::GetGraphicRenderer().GetRenderData();
        renderData.m_ViewMatrix = viewMatrix;
        renderData.m_ProjectionMatrix = projectionMatrix;
        renderData.m_CameraDirection = forward.Resize<3>();
        renderData.m_CameraPosition = transform.m_Translation;
        renderData.m_CameraJitter = cameraJitter;
        renderData.m_HdriTextureID = camera.GetHdriTextureID();

        // Only render the first camera for now, since the renderer is not designed for multiple yet
        break;
    }
}
