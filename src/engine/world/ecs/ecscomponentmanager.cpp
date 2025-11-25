/*
    This file is part of Ether, an open-source DirectX 12 renderer.

    Copyright (c) 2020-2030 Samuel Huang - All rights reserved.

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

#include "engine/world/ecs/ecscomponentmanager.h"

#include "engine/world/ecs/components/ecscameracomponent.h"
#include "engine/world/ecs/components/ecsmetadatacomponent.h"
#include "engine/world/ecs/components/ecstransformcomponent.h"
#include "engine/world/ecs/components/ecsvisualcomponent.h"

constexpr uint32_t EcsComponentManagerVersion = 0;

Ether::Ecs::EcsComponentManager::EcsComponentManager()
    : Serializable(EcsComponentManagerVersion, "Engine::EcsComponentManager")
    , m_NextID(0)
{
    RegisterComponent<EcsMetadataComponent>();
    RegisterComponent<EcsTransformComponent>();
    RegisterComponent<EcsCameraComponent>();
    RegisterComponent<EcsVisualComponent>();
    RegisterComponent<EcsSkinnedVisualComponent>();
}

void Ether::Ecs::EcsComponentManager::Serialize(OStream& ostream) const
{
    Serializable::Serialize(ostream);

    ostream << static_cast<uint32_t>(m_ComponentArrays.size());
    for (auto& pair : m_ComponentArrays)
    {
        ostream << static_cast<uint32_t>(pair.first);
        pair.second->Serialize(ostream);
    }
}

void Ether::Ecs::EcsComponentManager::Deserialize(IStream& istream)
{
    Reset();

    Serializable::Deserialize(istream);

    uint32_t numArrays;
    istream >> numArrays;

    for (int i = 0; i < numArrays; ++i)
    {
        uint32_t componentTypeID;
        istream >> componentTypeID;
        m_ComponentArrays[componentTypeID]->Deserialize(istream);
    }
}

void Ether::Ecs::EcsComponentManager::Reset()
{
    for (auto iter = m_ComponentArrays.begin(); iter != m_ComponentArrays.end(); ++iter)
    {
        iter->second->Reset();
    }
}

bool Ether::Ecs::EcsComponentManager::AddComponent(EntityID entityID, ComponentID id)
{
    if (!m_ComponentFactories.contains(id))
    {
        LogToolmodeError("Failed to add component - Component ID %u is not registered", id);
        return false;
    }

    m_ComponentFactories.at(id)(entityID);
    return true;
}

bool Ether::Ecs::EcsComponentManager::RemoveComponent(EntityID entityID, ComponentID id)
{
    if (!m_ComponentArrays.contains(id))
    {
        LogToolmodeError("Failed to remove component - Component ID %u is not registered", id);
        return false;
    }

    m_ComponentArrays.at(id)->RemoveComponent(entityID);
    return true;
}
