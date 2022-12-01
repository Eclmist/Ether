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

#pragma once

#include "pch.h"

namespace Ether
{
    class SceneGraphNode : public Serializable
    {
    public:
        SceneGraphNode();
        ~SceneGraphNode() override = default;

    public:
        void Serialize(OStream& ostream) override;
        void Deserialize(IStream& istream) override;

    private:
        friend class SceneGraph;

        EntityID m_ParentIndex;
        std::vector<EntityID> m_ChildrenIndices;
        bool m_IsRegistered;
    };

    class SceneGraph : public Serializable
    {
    public:
        SceneGraph();
        ~SceneGraph() override = default;

    public:
        void Serialize(OStream& ostream) override;
        void Deserialize(IStream& istream) override;

    public:
        inline EntityID GetParent(EntityID id) const { return m_Nodes[id].m_ParentIndex; }
        inline EntityID GetFirstChild(EntityID id) const { return m_Nodes[id].m_ChildrenIndices.front(); }
        inline EntityID GetLastChild(EntityID id) const { return m_Nodes[id].m_ChildrenIndices.back(); }
        inline const std::vector<EntityID>& GetChildren(EntityID id) const { return m_Nodes[id].m_ChildrenIndices; }

        void Register(EntityID id, EntityID parent = RootEntityID);
        void Deregister(EntityID id);
        void SetParent(EntityID id, EntityID parent);

    private:
        SceneGraphNode m_Nodes[MaxNumEntities];
    };
}

