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
#include "system/subsystem.h"
#include "engine/enginesubsystemid.h"

// Intended fancy ab(use) of macros
#define ESSID_NAME(name)                __EngineSubsystemID__##name
#define DEFINE_ENGINESUBSYSTEM(name)    namespace ENGINE_LINKSPACE { EngineSubsystemID ESSID_NAME(name)(#name, *(new name())); }
#define DECLARE_ENGINESUBSYSTEM(name)   namespace ENGINE_LINKSPACE { extern EngineSubsystemID ESSID_NAME(name); }
#define ENGINE_SUBSYSTEM(name)          static_cast<name*>(EngineSubsystemRegistry::GetInstance().GetSubsystem(*(ENGINE_LINKSPACE::ESSID_NAME(name))))

class EngineSubsystem : public Subsystem
{
public:
    EngineSubsystem() = default;
    ~EngineSubsystem() = default;
        
    virtual void Initialize() override { m_IsInitialized = true; };
    virtual void Shutdown() override { m_IsInitialized = false; };

public:
    inline void SetID(EngineSubsystemID& id) { m_ID = &id; }
    inline const EngineSubsystemID GetID() const { return *m_ID; };

private:
    EngineSubsystemID* m_ID;
};
