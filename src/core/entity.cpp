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

Entity::Entity(const std::wstring& name)
    : m_Name(name)
    , m_Parent(nullptr)
{
    m_Transform = AddComponent<TransformComponent>();
}

Entity::~Entity()
{
    for (Entity* child : m_Children)
        delete child;

    for (Component* component : m_Components)
        delete component;
}

void Entity::SetParent(Entity* parent)
{
    if (m_Parent == parent)
        return;

    if (m_Parent != nullptr)
        m_Parent->RemoveChild(this);

    if (parent != nullptr)
        parent->m_Children.push_back(this);

    m_Parent = parent;
}

void Entity::RemoveChild(Entity* child)
{
    for (auto iter = m_Children.begin(); iter != m_Children.end(); ++iter)
    {
        if (*iter != child)
            continue;

        m_Children.erase(iter);
        return;
    }
}

ETH_NAMESPACE_END

