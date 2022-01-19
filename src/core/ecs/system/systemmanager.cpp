/*
    This file is part of Ether, an open-source DirectX 12 renderer.

    Copyright (c) 2020-2022 Samuel Huang - All rights reserved.

    Ether is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "systemmanager.h"

#include "core/ecs/system/renderingsystem.h"

ETH_NAMESPACE_BEGIN

void SystemManager::AssignEntityToSystems(EntityID id, ComponentSignature signature)
{
    for (auto& pair : m_Systems)
    {
        auto& system = pair.second;
        auto systemSign = system->GetSignature();
        if ((signature & systemSign) == systemSign)
        {
            system->m_MatchingEntities.insert(id);
            system->OnEntityRegister(id);
        }
        else
        {
            system->OnEntityDeregister(id);
            system->m_MatchingEntities.erase(id);
        }
    }
}

void SystemManager::InitializeSystems()
{
    RegisterSystem<RenderingSystem>();
}

ETH_NAMESPACE_END

