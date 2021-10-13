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

#include "getcomponentscommand.h"

ETH_NAMESPACE_BEGIN

GetComponentsCommand::GetComponentsCommand(const CommandData& data)
    : m_EntityGuid(data["args"]["entityguid"])
{
    // TODO: Add support for type selection
}

void GetComponentsCommand::Execute()
{
    auto response = std::make_shared<GetComponentsCommandResponse>(m_EntityGuid);
    EngineCore::GetIpcManager().QueueCommand(response);
}

GetComponentsCommandResponse::GetComponentsCommandResponse(EntityID guid)
    : m_EntityGuid(guid)
{
}

std::string GetComponentsCommandResponse::GetSendableData() const
{
    //CommandData command;
    //command["command"] = "getcomponents";

    //auto entity = EngineCore::GetECSManager().GetEntity(m_EntityGuid);
    ////auto signature = entity->GetComponentSignature();

    ////for (int i = 0; i < ETH_ECS_MAX_COMPONENTS; ++i)
    ////{
    ////    if (signature.test(i))
    ////    {

    //auto components = entity->GetComponents();

    //for (int i = 0; i < components.size(); ++i)
    //{
    //    //command["args"]["components"][i]["guid"] = component->GetGuid();
    //    //command["args"]["components"][i]["type"] = components[i].;
    //    //command["args"]["components"][i]["entityGuid"] = components[i]->GetOwner();
    //    //command["args"]["components"][i]["enabled"] = true;// component->GetGuid(); TODO
    //}

    //    }
    //}
    


    //for (int i = 0; i < topLevelEntities.size(); ++i)
    //{
    //    command["args"]["entities"][i]["name"] = EngineCore::GetECSManager().GetEntity(topLevelEntities[i])->GetName();
    //    command["args"]["entities"][i]["guid"] = std::to_string(EngineCore::GetECSManager().GetEntity(topLevelEntities[i])->GetID());
    //    command["args"]["entities"][i]["parent"] = "";
    //    command["args"]["entities"][i]["enabled"] = EngineCore::GetECSManager().GetEntity(topLevelEntities[i])->IsEnabled();
    //}
    //return command.dump();

    return "";
}

ETH_NAMESPACE_END

