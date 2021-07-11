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

class ETH_ENGINE_DLL Entity // : public Serializable?
{
public:
    Entity(const std::wstring& name);
    ~Entity();

public:
    inline Entity* GetParent() const { return m_Parent != nullptr ? m_Parent : nullptr; }
    inline Entity* GetFirstChild() const { return !m_Children.empty() ? m_Children[0] : nullptr; }
    inline Entity* GetLastChild() const { return !m_Children.empty() ? m_Children[m_Children.size() - 1] : nullptr; }
    inline size_t GetNumChildren() const { return m_Children.size(); }
    inline Entity* GetChildren() const { return *m_Children.data(); }
    void SetParent(Entity* parent);

public:
    template <typename T>
    T* AddComponent()
    {
        T* newComponent = new T(this);
        m_Components.push_back(newComponent);
        return newComponent;
    }

    template <typename T>
    T* GetComponent()
    {
        for (auto component : m_Components)
        {
            T* targetComponent = dynamic_cast<T*>(component);
            if (targetComponent != nullptr)
                return targetComponent;
        }

        return nullptr;
    }

    template <typename T>
    void GetComponents(T* outArr, size_t numComponents)
    {
        for (auto component : m_Components)
        {
            T* targetComponent = dynamic_cast<T*>(component);
            if (targetComponent != nullptr)
            {
                outArr = targetComponent;
                outArr++;
            }
        }
    }

    TransformComponent* GetTransform() const { return m_Transform; }

private:
    void RemoveChild(Entity* child);

private:
    std::wstring m_Name;

    Entity* m_Parent;
    std::vector<Entity*> m_Children;
    std::vector<Component*> m_Components;

private:
    TransformComponent* m_Transform;
};

ETH_NAMESPACE_END

