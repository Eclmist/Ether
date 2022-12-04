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

#include "engine/world/scenegraph.h"

constexpr uint32_t SceneGraphVersion = 0;

Ether::SceneGraphNode::SceneGraphNode()
    : Serializable(SceneGraphVersion, StringID("Engine::SceneGraphNode").GetHash())
    , m_ParentIndex(InvalidEntityID)
    , m_IsRegistered(false)
{
}

void Ether::SceneGraphNode::Serialize(OStream& ostream) const
{
    Serializable::Serialize(ostream);

    ostream << m_ParentIndex;
    ostream << m_IsRegistered;
    ostream << (uint32_t)m_ChildrenIndices.size();

    for (int i = 0; i < m_ChildrenIndices.size(); ++i)
        ostream << m_ChildrenIndices[i];
}

void Ether::SceneGraphNode::Deserialize(IStream& istream)
{
    Serializable::Deserialize(istream);

    istream >> m_ParentIndex;
    istream >> m_IsRegistered;

    uint32_t numIndices;
    istream >> numIndices;

    for (int i = 0; i < numIndices; ++i)
        istream >> m_ChildrenIndices[i];
}

Ether::SceneGraph::SceneGraph()
    : Serializable(SceneGraphVersion, StringID("Engine::SceneGraph").GetHash())
{
    m_Nodes[RootEntityID].m_IsRegistered = true;
    m_Nodes[RootEntityID].m_ParentIndex = InvalidEntityID;
}

void Ether::SceneGraph::Serialize(OStream& ostream) const
{
    Serializable::Serialize(ostream);

    for (int i = 0; i < Ecs::MaxNumEntities; ++i)
        m_Nodes[i].Serialize(ostream);
}

void Ether::SceneGraph::Deserialize(IStream& istream)
{
    Serializable::Deserialize(istream);

    for (int i = 0; i < Ecs::MaxNumEntities; ++i)
        m_Nodes[i].Deserialize(istream);
}

void Ether::SceneGraph::SetParent(Ecs::EntityID id, Ecs::EntityID parent)
{
    if (m_Nodes[id].m_ParentIndex != RootEntityID)
    {
        SceneGraphNode oldParent = m_Nodes[m_Nodes[id].m_ParentIndex];

        oldParent.m_ChildrenIndices.erase(
            std::remove(oldParent.m_ChildrenIndices.begin(), oldParent.m_ChildrenIndices.end(), id),
            oldParent.m_ChildrenIndices.end()
        );
    }

    m_Nodes[id].m_ParentIndex = parent;
}

void Ether::SceneGraph::Register(Ecs::EntityID id, Ecs::EntityID parent)
{
    AssertEngine(!m_Nodes[id].m_IsRegistered, "EntityID is already registered to the scene graph");
    m_Nodes[id].m_IsRegistered = true;
    m_Nodes[id].m_ParentIndex = parent;
    m_Nodes[parent].m_ChildrenIndices.push_back(id);
}

void Ether::SceneGraph::Deregister(Ecs::EntityID id)
{
    AssertEngine(m_Nodes[id].m_IsRegistered, "EntityID was never registered to the scene graph");
    m_Nodes[id].m_IsRegistered = false;
    SetParent(id, RootEntityID);

    for (Ecs::EntityID childIdx : m_Nodes[id].m_ChildrenIndices)
        m_Nodes[childIdx].m_ParentIndex = RootEntityID;

    m_Nodes[id].m_ChildrenIndices.clear();
}
