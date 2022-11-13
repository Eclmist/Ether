/*
    This file is part of Ether, an open-source DirectX 12 renderer.

    Copyright (c) 2020-2023 Samuel Huang - All rights reserved.

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

GetComponentsCommandResponse::GetComponentsCommandResponse(const std::string& guid)
    : m_EntityGuid(guid)
{
}

std::string GetComponentsCommandResponse::GetSendableData() const
{
    CommandData command;
    command["command"] = "getcomponents";

    EntityID entityID = EngineCore::GetEcsManager().GetEntityID(m_EntityGuid);
    Entity* entity = EngineCore::GetEcsManager().GetEntity(entityID);
    Component** components = entity->m_Components;

    for (int i = 0; i < ETH_ECS_MAX_COMPONENTS; ++i)
    {
        if (components[i] == nullptr)
            continue;

        command["args"]["components"][i]["name"] = components[i]->GetName();
        command["args"]["components"][i]["guid"] = components[i]->GetGuid();
        command["args"]["components"][i]["entity_guid"] = m_EntityGuid;
        command["args"]["components"][i]["enabled"] = components[i]->IsEnabled();

        const auto fields = components[i]->GetInspectorFields();
        for (int j = 0; j < fields.size(); ++j)
            command["args"]["components"][i]["fields"][j] = CommandData::parse(fields[j]->GetData());
    }

    return command.dump();
}

ETH_NAMESPACE_END

