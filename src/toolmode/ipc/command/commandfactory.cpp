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

#pragma once

#include "commandfactory.h"

#include "initializecommand.h"
#include "detachcommand.h"
#include "resizecommand.h"

#include "asset/importassetcommand.h"

#include "ecs/gettoplevelentitiescommand.h"
#include "ecs/getcomponentscommand.h"
#include "ecs/setcomponentcommand.h"

#include "state/viewport/setdrawmodecommand.h"


ETH_NAMESPACE_BEGIN

#define REGISTER_COMMAND(id, T) RegisterCommand(id, [](const CommandData& c) { return std::make_unique<T>(c); })

CommandFactory::CommandFactory()
{
    REGISTER_COMMAND("initialize", InitializeCommand);
    REGISTER_COMMAND("detach", DetachCommand);
    REGISTER_COMMAND("resize", ResizeCommand);

    // Asset
    REGISTER_COMMAND("importasset", ImportAssetCommand);

    // ECS
    REGISTER_COMMAND("gettoplevelentities", GetTopLevelEntitiesCommand);
    REGISTER_COMMAND("getcomponents", GetComponentsCommand);
    REGISTER_COMMAND("setcomponent", SetComponentCommand);

    // State
    REGISTER_COMMAND("setdrawmode", SetDrawModeCommand);
}

std::unique_ptr<Command> CommandFactory::CreateCommand(const std::string& commandID, const CommandData& data) const
{
    if (m_FactoryMap.find(commandID) == m_FactoryMap.end())
        return nullptr;

    return (m_FactoryMap.find(commandID)->second)(data);
}

void CommandFactory::RegisterCommand(const std::string& commandID, FactoryFunction factoryFunction)
{
    if (m_FactoryMap.find(commandID) != m_FactoryMap.end())
    {
        LogToolmodeError("The command %s has already been registered", commandID);
        return;
    }

    m_FactoryMap[commandID] = factoryFunction;
}

ETH_NAMESPACE_END

