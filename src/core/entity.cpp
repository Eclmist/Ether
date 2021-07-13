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

#include "entity.h"

ETH_NAMESPACE_BEGIN

Entity::Entity(const std::string& name)
    : m_Name(name)
    , m_Parent(nullptr)
    , m_Transform(*AddComponent<TransformComponent>())
{
}

Entity::~Entity()
{
}

void Entity::SetParent(Entity& parent)
{
    if (m_Parent == &parent)
        return;

    // While all entity should have a parent (except World::m_RootEntity), during the initial
    // construction of entity it may still not have a parent.
    if (m_Parent != nullptr)
        m_Parent->RemoveChild(*this);

    parent.m_Children.emplace_back(this);
    m_Parent = &parent;
}

void Entity::RemoveChild(const Entity& child)
{
    for (auto iter = m_Children.begin(); iter != m_Children.end(); ++iter)
    {
        if ((*iter).get() != &child)
            continue;

        (*iter).release();
        m_Children.erase(iter);
        return;
    }
}

ETH_NAMESPACE_END

