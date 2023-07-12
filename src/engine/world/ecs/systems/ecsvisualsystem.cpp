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
#include "engine/world/ecs/systems/ecsvisualsystem.h"
#include "engine/world/ecs/components/ecsvisualcomponent.h"

#include "graphics/graphiccore.h"
#include "graphics/common/visualbatch.h"

Ether::Ecs::EcsVisualSystem::EcsVisualSystem()
{
    m_Signature.set(EcsVisualComponent::s_ComponentID);
}

void Ether::Ecs::EcsVisualSystem::Update()
{
    ETH_MARKER_EVENT("Visual System - Update");

    Graphics::VisualBatch visualBatch;

    ResourceManager& resources = EngineCore::GetActiveWorld().GetResourceManager();

    for (EntityID entityID : m_Entities)
    {
        Entity& entity = EngineCore::GetActiveWorld().GetEntity(entityID);
        EcsVisualComponent& data = entity.GetComponent<EcsVisualComponent>();

        if (!data.m_Enabled)
            continue;

        Graphics::Visual gfxVisual;
        gfxVisual.m_ParentBatch = &visualBatch;
        gfxVisual.m_Mesh = resources.GetMeshResource(data.m_MeshGuid);

        if (gfxVisual.m_Mesh == nullptr)
            continue;

        if (data.m_MaterialGuid == StringID(""))
            gfxVisual.m_Material = Graphics::GraphicCore::GetGraphicCommon().m_DefaultMaterial.get();
        else
            gfxVisual.m_Material = resources.GetMaterialResource(data.m_MaterialGuid);

        if (gfxVisual.m_Material == nullptr)
            gfxVisual.m_Material = Graphics::GraphicCore::GetGraphicCommon().m_ErrorMaterial.get();

        if (gfxVisual.m_Material->GetBaseColor().w < 1.0)
            continue; // Don't support transparency

        visualBatch.m_Visuals.emplace_back(gfxVisual);
    }

    Graphics::RenderData& renderData = Graphics::GraphicCore::GetGraphicRenderer().GetRenderData();
    renderData.m_VisualBatch = visualBatch;
}
