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

class ETH_ENGINE_DLL Entity : public Serializable
{
public:
    Entity(const EntityID id, const std::string& name);
    ~Entity();

public:
    inline const EntityID GetID() const { return m_ID; }

    inline const std::string& GetName() const { return m_Name; }
    inline void SetName(const std::string& name) { m_Name = name; }

    inline ComponentSignature GetComponentSignature() const { return m_ComponentSignature; }
    inline void SetComponentSignature(ComponentID id, bool isSet) { m_ComponentSignature.set(id, isSet); }

    inline bool IsEnabled() const { return m_Enabled; }
    inline void SetEnabled(bool enabled) { m_Enabled = enabled; }

public:
    virtual void Serialize(OStream& ostream) override;
    virtual void Deserialize(IStream& istream) override;

public:
    template <typename T>
    T* const GetComponent() const
    {
        return EngineCore::GetECSManager().GetComponent<T>(m_ID);
    }

    template <typename T>
    T* const AddComponent() const
    {
        return EngineCore::GetECSManager().AddComponent<T>(m_ID);
    }

    template <typename T>
    T* const RemoveComponent() const
    {
        return EngineCore::GetECSManager().RemoveComponent()<T>(m_ID);
    }

public:
    ETH_TOOLONLY(Component* m_Components[ETH_ECS_MAX_COMPONENTS]);

private:
    friend class EntityManager;
    void AddMandatoryComponents();

private:
    EntityID m_ID;
    std::string m_Name;
    bool m_Enabled;

    ComponentSignature m_ComponentSignature;
};

ETH_NAMESPACE_END

