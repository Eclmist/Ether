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
#include "core/entity/component/transformcomponent.h"

ETH_NAMESPACE_BEGIN

Entity::Entity(const std::wstring& name)
    : m_Name(name)
    , m_Parent(nullptr)
{
    m_Transform = new TransformComponent();
    AddComponent(m_Transform);
}

Entity::~Entity()
{
    for (Entity* child : m_Children)
        delete child;

    for (Component* component : m_Components)
        delete component;
}

Entity* Entity::GetFirstChild() const
{
    if (!HasChildren())
        return nullptr;

    return m_Children.front();
}

Entity* Entity::GetChildAtIndex(uint32_t i) const
{
    if (i >= GetNumChildren())
        return nullptr;

    return m_Children[i];
}

bool Entity::IsChildOf(Entity* parent)
{
    for (Entity* child : parent->m_Children)
        if (child == this)
            return true;

    return false;
}

void Entity::SetParent(Entity* parent)
{
    AssertEngine(parent != nullptr, "SetParent was called with a nullptr");

    if (IsChildOf(parent))
        return;

    if (m_Parent != nullptr)
        m_Parent->RemoveChild(this);

    parent->m_Children.push_back(this);
    m_Parent = parent;
}

void Entity::RemoveChild(Entity* child)
{
    if (!child->IsChildOf(this))
    {
        LogEngineWarning("Failed to remove entity %s from %s: not parent-child", child->m_Name, m_Name);
        return;
    }

    for (auto iter = m_Children.begin(); iter != m_Children.end(); ++iter)
    {
        if (*iter != child)
            continue;

        m_Children.erase(iter);
        return;
    }

    LogEngineError("Entity::HasChild() check failed (Entity::RemoveChild)");
}

void Entity::AddComponent(Component* component)
{
    m_Components.push_back(component);
}

ETH_NAMESPACE_END

