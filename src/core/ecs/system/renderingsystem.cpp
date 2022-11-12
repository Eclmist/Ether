/*
    This file is part of Ether, an open-source DirectX 12 renderer.

    Copyright (c) 2020-2022 Samuel Huang - All rights reserved.

    Ether is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "renderingsystem.h"

ETH_NAMESPACE_BEGIN

RenderingSystem::RenderingSystem()
{
    m_Signature.set(EngineCore::GetEcsManager().GetComponentID<TransformComponent>());
    m_Signature.set(EngineCore::GetEcsManager().GetComponentID<MeshComponent>());
    m_Signature.set(EngineCore::GetEcsManager().GetComponentID<VisualComponent>());
}

void RenderingSystem::OnEntityRegister(EntityID id)
{
}

void RenderingSystem::OnEntityDeregister(EntityID id)
{
    m_VisualNodes.erase(id);
}

void RenderingSystem::OnSceneLoad()
{
    OPTICK_EVENT("ECS - Rendering System - OnSceneLoad");

    for (EntityID id : m_MatchingEntities)
    {
        auto* visual = EngineCore::GetEcsManager().GetComponent<VisualComponent>(id);

        for (auto pair : visual->GetMaterial()->m_Textures)
        {
            std::shared_ptr<CompiledTexture> texture = pair.second;

            if (texture == nullptr)
                continue;

            if (texture->GetDepth() == 1)
                GraphicCore::GetGraphicRenderer().GetResourceContext().InitializeTexture2D(*texture);
        }
    }
}

void RenderingSystem::OnUpdate()
{
    OPTICK_EVENT("ECS - Rendering System - Update");

    bool hasMeshChanges = false;

    for (EntityID id : m_MatchingEntities)
    {
        OPTICK_EVENT("ECS - Rendering System - Visual Node Validation");

        auto* mesh = EngineCore::GetEcsManager().GetComponent<MeshComponent>(id);
        auto* transform = EngineCore::GetEcsManager().GetComponent<TransformComponent>(id);
        auto* visual = EngineCore::GetEcsManager().GetComponent<VisualComponent>(id);

        if (!mesh->IsEnabled())
            return;

        if (!mesh->HasMesh())
            return;

        if (!mesh->GetCompiledMesh()->IsValid())
            return;

        // TODO: Right now visual nodes are recreated when mesh changes -> This is because
        // on the user level you should be able to swap out meshes of particular mesh components.
        // However, the ECS "registration" mechanism works on a component level. If some mechanism
        // like "reregister entity" can be added, then we don't need the following check and
        // visuals can be created in "onregister" once only.
        if (mesh->IsMeshChanged())
        {
            VisualNodeDesc data;
            data.m_NodeId = id;
            data.m_VertexBuffer = mesh->GetCompiledMesh()->GetVertexBuffer();
            data.m_NumVertices = mesh->GetCompiledMesh()->GetNumVertices();
            data.m_ModelMatrix = transform->GetMatrixReference();
            data.m_Material = visual->GetMaterial();
            ETH_TOOLONLY(data.m_PickerColor = EngineCore::GetEcsManager().GetPickierColorFromID(id));

            m_VisualNodes[id] = std::make_unique<VisualNode>(data);
            mesh->SetMeshChanged(false);
            hasMeshChanges = true;
        }

        // TODO: Add proper place to upload data - the current implementation in bufferresource is incorrect.
        //if (hasMeshChanges)
        //{
        //    GraphicCore::GetGraphicRenderer().GetGraphicContext().
        //}

        GraphicCore::GetGraphicRenderer().DrawNode(m_VisualNodes[id].get());
    }
}

ETH_NAMESPACE_END

