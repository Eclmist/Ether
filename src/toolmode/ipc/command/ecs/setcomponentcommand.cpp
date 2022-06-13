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

#include "setcomponentcommand.h"

ETH_NAMESPACE_BEGIN

SetComponentCommand::SetComponentCommand(const CommandData& data)
    : m_ComponentData(data["args"]["component"])
{
}

void SetComponentCommand::Execute()
{
    Component* component = EngineCore::GetECSManager().GetComponentByGuid(m_ComponentData["guid"]);

    std::vector<std::shared_ptr<Field>> fields = component->GetInspectorFields();
    std::unordered_map<std::string, std::shared_ptr<Field>> nameToFieldMap;
    auto properties = m_ComponentData["fields"];

    for (int i = 0; i < fields.size(); ++i)
        nameToFieldMap[fields[i]->GetName()] = fields[i];

    for (int i = 0; i < properties.size(); ++i)
        if (nameToFieldMap.find(properties[i]["name"]) != nameToFieldMap.end())
            nameToFieldMap[properties[i]["name"]]->SetData(properties[i]);
}

ETH_NAMESPACE_END

