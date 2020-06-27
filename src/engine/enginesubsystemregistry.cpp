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

#include "enginesubsystemregistry.h"

EngineSubsystemRegistry::~EngineSubsystemRegistry()
{
    for (auto subsystem : m_Subsystems)
        delete subsystem;

    m_Subsystems.clear();
}

EngineSubsystemIndex EngineSubsystemRegistry::Register(EngineSubsystem& subsystem)
{
    assert(std::find(m_Subsystems.begin(), m_Subsystems.end(), &subsystem) == m_Subsystems.end() &&
        "This subsystem has already been registered.");

    m_Subsystems.push_back(&subsystem);
    m_NumRegisteredSubsystems++;
    return static_cast<EngineSubsystemIndex>(m_Subsystems.size());
}

void EngineSubsystemRegistry::Unregister(EngineSubsystem& subsystem)
{
    assert(m_Subsystems[*subsystem.GetID()] != nullptr &&
        "This subsystem has not yet been registered.");

    m_Subsystems[*subsystem.GetID()] = nullptr;
    m_NumRegisteredSubsystems--;
}

EngineSubsystem* EngineSubsystemRegistry::GetSubsystem(const EngineSubsystemIndex& index)
{
    if (index >= m_Subsystems.size())
        return nullptr;

    return m_Subsystems[index];
}

std::vector<EngineSubsystem*> EngineSubsystemRegistry::GetRegisteredSubsystems() const
{
    std::vector<EngineSubsystem*> registeredSubsystems(m_NumRegisteredSubsystems);

    for (int i = 0; i < m_Subsystems.size(); ++i)
    {
        if (m_Subsystems[i] != nullptr)
            registeredSubsystems.push_back(m_Subsystems[i]);
    }

    assert(registeredSubsystems.size() == m_NumRegisteredSubsystems);

    return registeredSubsystems;
}
