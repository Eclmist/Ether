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

ETH_NAMESPACE_BEGIN

class ETH_ENGINE_DLL Entity : public NonCopyable // : public Serializable?
{
public:
    Entity(const std::string& name);
    ~Entity();

public:
    inline const std::string& GetName() const { return m_Name; }
    inline void SetName(const std::string& name) { m_Name = name; }

    inline Entity* const GetParent() { return m_Parent != nullptr ? m_Parent : nullptr; }
    void SetParent(Entity& parent);

    inline Entity* const GetFirstChild() { return !m_Children.empty() ? m_Children[0].get() : nullptr; }
    inline Entity* const GetLastChild() { return !m_Children.empty() ? m_Children[m_Children.size() - 1].get() : nullptr; }
    inline std::vector<std::unique_ptr<Entity>>& GetChildren() { return m_Children; }

public:
    template <typename T>
    T* const AddComponent()
    {
        m_Components.emplace_back(std::make_unique<T>(*this));
        return dynamic_cast<T*>(m_Components.back().get());
    }

    template <typename T>
    T* const GetComponent()
    {
        for (auto&& component : m_Components)
        {
            T* targetComponent = dynamic_cast<T*>(component.get());
            if (targetComponent != nullptr)
                return targetComponent;
        }

        return nullptr;
    }

    TransformComponent& GetTransform() { return m_Transform; }

private:
    void RemoveChild(const Entity& child);

private:
    std::string m_Name;

    Entity* m_Parent;
    std::vector<std::unique_ptr<Entity>> m_Children;
    std::vector<std::unique_ptr<Component>> m_Components;

private:
    TransformComponent& m_Transform;
};

ETH_NAMESPACE_END

