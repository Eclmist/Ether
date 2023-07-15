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
#include "engine/world/ecs/components/ecscameracomponent.h"
#include "engine/world/ecs/components/ecstransformcomponent.h"

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

            gfxVisualBatch->m_Material->SetTransientMaterialIdx(materialToBatchMap.at(data.m_MaterialGuid));
        }
        else
        {
            gfxVisualBatch = &renderData.m_VisualBatches[materialToBatchMap.at(data.m_MaterialGuid)];
        }

        if (gfxVisualBatch->m_Material->GetBaseColor().w < 1.0)
            continue; // Don't support transparency


        gfxVisual.m_Mesh = resources.GetMeshResource(data.m_MeshGuid);
        if (gfxVisual.m_Mesh == nullptr)
            continue;

        gfxVisual.m_Material = gfxVisualBatch->m_Material;
        gfxVisual.m_Culled = !IsVisualCulled(gfxVisual);

        renderData.m_Visuals.push_back(gfxVisual);
        gfxVisualBatch->m_Visuals.emplace_back(gfxVisual);
    }
}

bool Ether::Ecs::EcsVisualSystem::IsVisualCulled(const Graphics::Visual& visual) const
{
    Entity* camera = EngineCore::GetActiveWorld().GetMainCamera();
    if (camera == nullptr)
        return false;

    Graphics::RenderData& renderData = Graphics::GraphicCore::GetGraphicRenderer().GetRenderData();

    Aabb visualAabb = visual.m_Mesh->GetBoundingBox();

    ethMatrix4x4 viewProjectionMatrix = renderData.m_ProjectionMatrix * renderData.m_ViewMatrix;
    //viewProjectionMatrix = viewProjectionMatrix.Transposed();

    ethVector4 planes[6];
    planes[0] = ethVector4(viewProjectionMatrix.m_Data2D[2]);
    planes[1] = ethVector4(viewProjectionMatrix.m_Data2D[3]) - ethVector4(viewProjectionMatrix.m_Data2D[2]);
    planes[2] = ethVector4(viewProjectionMatrix.m_Data2D[3]) + ethVector4(viewProjectionMatrix.m_Data2D[0]);
    planes[3] = ethVector4(viewProjectionMatrix.m_Data2D[3]) - ethVector4(viewProjectionMatrix.m_Data2D[0]);
    planes[4] = ethVector4(viewProjectionMatrix.m_Data2D[3]) - ethVector4(viewProjectionMatrix.m_Data2D[1]);
    planes[5] = ethVector4(viewProjectionMatrix.m_Data2D[3]) + ethVector4(viewProjectionMatrix.m_Data2D[1]);

    for (uint32_t i = 0; i < 6; ++i)
    {
        ethVector3 normal = planes[i].Resize<3>();
        planes[i] /= normal.Magnitude();
    }

    for (uint32_t i = 0; i < 6; ++i)
    {
        ethVector4 vert;
        vert.x = planes[i].x > 0 ? visualAabb.m_Max.x : visualAabb.m_Min.x;
        vert.y = planes[i].y > 0 ? visualAabb.m_Max.y : visualAabb.m_Min.y;
        vert.z = planes[i].z > 0 ? visualAabb.m_Max.z : visualAabb.m_Min.z;
        vert.w = 1.0f;

        if (ethVector4::Dot(vert, planes[i]) < 0.0f)
            return false;
    }
    return true;
}
