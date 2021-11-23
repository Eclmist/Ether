/*
    This file is part of Ether, an open-source DirectX 12 renderer.

    Copyright (c) 2020-2021 Samuel Van Allen - All rights reserved.

    Ether is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "renderingsystem.h"

ETH_NAMESPACE_BEGIN

RenderingSystem::RenderingSystem()
{
    m_Signature.set(EngineCore::GetECSManager().GetComponentID<TransformComponent>(), true);
    m_Signature.set(EngineCore::GetECSManager().GetComponentID<MeshComponent>(), true);
    m_Signature.set(EngineCore::GetECSManager().GetComponentID<VisualComponent>(), true);
}

void RenderingSystem::OnEntityRegister(EntityID id)
{
}

void RenderingSystem::OnEntityDeregister(EntityID id)
{
    m_VisualNodes.erase(id);
}

void RenderingSystem::OnUpdate()
{
    OPTICK_EVENT("ECS - Rendering System - Update");

    for (EntityID id : m_MatchingEntities)
    {
		auto* mesh = EngineCore::GetECSManager().GetComponent<MeshComponent>(id);
		auto* transform = EngineCore::GetECSManager().GetComponent<TransformComponent>(id);

		if (!mesh->IsEnabled())
			return;

		if (!mesh->HasMesh())
			return;

        if (!mesh->GetMeshAsset()->IsValid())
            return;

        if (mesh->IsMeshChanged())
        {
            VisualNodeData data;
            data.m_VertexBuffer = mesh->GetVertexBuffer();
            data.m_IndexBuffer = mesh->GetIndexBuffer();
            data.m_NumIndices = mesh->GetNumIndices();
            data.m_NumVertices = mesh->GetNumVertices();
            data.m_ModelMatrix = transform->GetMatrixReference();
            m_VisualNodes[id] = std::make_unique<VisualNode>(data);
            mesh->SetMeshChanged(false);
        }

        GraphicCore::GetGraphicRenderer().DrawNode(m_VisualNodes[id].get());
    }
}

ETH_NAMESPACE_END

