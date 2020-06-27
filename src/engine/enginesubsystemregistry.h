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

#include "system/system.h"
#include "engine/enginesubsystem.h"

class EngineSubsystemRegistry : public Singleton<EngineSubsystemRegistry>
{
public:
    EngineSubsystemRegistry() = default;
    ~EngineSubsystemRegistry();

public:
    EngineSubsystemIndex Register(EngineSubsystem& subsystem);
    void Unregister(EngineSubsystem& subsystem);

    EngineSubsystem* GetSubsystem(const EngineSubsystemIndex& index);
    std::vector<EngineSubsystem*> GetRegisteredSubsystems() const;

private:
    std::vector<EngineSubsystem*> m_Subsystems;
    uint32_t m_NumRegisteredSubsystems;
};
