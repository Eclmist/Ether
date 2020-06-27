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

typedef uint16_t EngineSubsystemIndex;

class EngineSubsystem;

class EngineSubsystemID
{
public:
    EngineSubsystemID(const char* name, EngineSubsystem& subsystem);

    inline const char* GetName() const { return m_Name; };
    inline const EngineSubsystemIndex operator*() const { return m_RegistryIndex; };

private:
    const char* m_Name;
    EngineSubsystemIndex m_RegistryIndex;
};
