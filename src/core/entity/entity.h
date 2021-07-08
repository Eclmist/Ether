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

#pragma once

#include "core/entity/component/component.h"
#include "core/entity/component/transformcomponent.h"

ETH_NAMESPACE_BEGIN

// TODO: Clean this mess

class ETH_ENGINE_DLL Entity // : public Serializable?
{
public:
    Entity(const std::wstring& name);
    ~Entity();

    inline Entity* GetParent() const { return m_Parent; }
    inline uint32_t GetNumChildren() const { return (uint32_t)m_Children.size(); }
    inline bool HasChildren() const { return !m_Children.empty(); }
    inline std::vector<Entity*> GetChildren() const { return m_Children; }

    Entity* GetFirstChild() const;
    Entity* GetChildAtIndex(uint32_t i) const;

    bool IsChildOf(Entity* parent);
    void SetParent(Entity* parent);
    void RemoveChild(Entity* child);

    void AddComponent(Component* component);

    // TODO: Change to get components of type
    std::vector<Component*> GetAllComponents() { return m_Components; }

    inline TransformComponent* GetTransform() const { return m_Transform; }

private:
    Entity* m_Parent;
    std::vector<Entity*> m_Children;
    std::vector<Component*> m_Components;

    std::wstring m_Name;

private:
    TransformComponent* m_Transform;
};

ETH_NAMESPACE_END
