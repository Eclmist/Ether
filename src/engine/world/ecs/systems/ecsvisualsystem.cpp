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

    ResourceManager& resources = EngineCore::GetActiveWorld().GetResourceManager();
    Graphics::RenderData& renderData = Graphics::GraphicCore::GetGraphicRenderer().GetRenderData();
    renderData.m_Visuals.clear();
    renderData.m_VisualBatches.clear();
    std::unordered_map<StringID, uint32_t> materialToBatchMap;

    for (EntityID entityID : m_Entities)
    {
        Entity& entity = EngineCore::GetActiveWorld().GetEntity(entityID);
        EcsVisualComponent& data = entity.GetComponent<EcsVisualComponent>();

        if (!data.m_Enabled)
            continue;

        Graphics::Visual gfxVisual;
        Graphics::VisualBatch* gfxVisualBatch;

        if (materialToBatchMap.find(data.m_MaterialGuid) == materialToBatchMap.end())
        {
            renderData.m_VisualBatches.emplace_back();
            materialToBatchMap[data.m_MaterialGuid] = renderData.m_VisualBatches.size() - 1;
            gfxVisualBatch = &renderData.m_VisualBatches[materialToBatchMap.at(data.m_MaterialGuid)];
            if (data.m_MaterialGuid == StringID(""))
            {
                gfxVisualBatch->m_Material = Graphics::GraphicCore::GetGraphicCommon().m_DefaultMaterial.get();
            }
            else
            {
                gfxVisualBatch->m_Material = resources.GetMaterialResource(data.m_MaterialGuid);

                if (gfxVisualBatch->m_Material == nullptr)
                    gfxVisualBatch->m_Material = Graphics::GraphicCore::GetGraphicCommon().m_ErrorMaterial.get();
            }

        }
        else
        {
            gfxVisualBatch = &renderData.m_VisualBatches[materialToBatchMap.at(data.m_MaterialGuid)];
        }

        if (gfxVisualBatch->m_Material->GetBaseColor().w < 1.0)
            continue; // Don't support transparency

        gfxVisual.m_Mesh = resources.GetMeshResource(data.m_MeshGuid);
        gfxVisual.m_Material = gfxVisualBatch->m_Material;

        if (gfxVisual.m_Mesh == nullptr)
            continue;

        renderData.m_Visuals.push_back(gfxVisual);
        gfxVisualBatch->m_Visuals.emplace_back(gfxVisual);
    }
}
