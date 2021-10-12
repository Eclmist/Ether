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

#include "scenegraph.h"

ETH_NAMESPACE_BEGIN

SceneGraph::SceneGraph()
{
    m_Nodes[ETH_ECS_ROOT_ENTITY_ID].m_IsRegistered = true;
    m_Nodes[ETH_ECS_ROOT_ENTITY_ID].m_ParentIndex = ETH_ECS_INVALID_ID;
}

void SceneGraph::SetParent(EntityID id, EntityID parent)
{
    if (m_Nodes[id].m_ParentIndex != ETH_ECS_ROOT_ENTITY_ID)
    {
        SceneGraphNode oldParent = m_Nodes[m_Nodes[id].m_ParentIndex];

        oldParent.m_ChildrenIndices.erase(
            std::remove(oldParent.m_ChildrenIndices.begin(), oldParent.m_ChildrenIndices.end(), id),
            oldParent.m_ChildrenIndices.end()
        );
    }

    m_Nodes[id].m_ParentIndex = parent;
}

void SceneGraph::Register(EntityID id, EntityID parent)
{
    AssertEngine(!m_Nodes[id].m_IsRegistered, "EntityID is already registered to the scene graph");
    m_Nodes[id].m_IsRegistered = true;
    m_Nodes[id].m_ParentIndex = parent;
    m_Nodes[parent].m_ChildrenIndices.push_back(id);
}

void SceneGraph::Deregister(EntityID id)
{
    AssertEngine(m_Nodes[id].m_IsRegistered, "EntityID was never registered to the scene graph");
    m_Nodes[id].m_IsRegistered = false;
    SetParent(id, ETH_ECS_ROOT_ENTITY_ID);

    for (EntityID childIdx : m_Nodes[id].m_ChildrenIndices)
        m_Nodes[childIdx].m_ParentIndex = ETH_ECS_ROOT_ENTITY_ID;

    m_Nodes[id].m_ChildrenIndices.clear();
}

ETH_NAMESPACE_END

