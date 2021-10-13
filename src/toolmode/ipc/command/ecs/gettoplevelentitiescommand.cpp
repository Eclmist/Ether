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

#include "gettoplevelentitiescommand.h"

ETH_NAMESPACE_BEGIN

GetTopLevelEntitiesCommand::GetTopLevelEntitiesCommand(const CommandData& data)
{
}

void GetTopLevelEntitiesCommand::Execute()
{
    auto response = std::make_shared<GetTopLevelEntitiesCommandResponse>();
    EngineCore::GetIpcManager().QueueCommand(response);
}

GetTopLevelEntitiesCommandResponse::GetTopLevelEntitiesCommandResponse()
{
}

std::string GetTopLevelEntitiesCommandResponse::GetSendableData() const
{
    CommandData command;
    auto topLevelEntities = EngineCore::GetActiveWorld().GetSceneGraph().GetChildren(ETH_ECS_ROOT_ENTITY_ID);
    for (int i = 0; i < topLevelEntities.size(); ++i)
        command["name"][i] = EngineCore::GetECSManager().GetEntity(topLevelEntities[i])->GetName();
    return command;
}

ETH_NAMESPACE_END
