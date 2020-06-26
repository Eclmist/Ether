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

#include "system.h"
#include "subsystemid.h"
#include "subsystemscheduler.h"

// Intended fancy (ab)use of macros
#define SUBSYSTEM_NAME(name)            __SubSystemID__##name
#define DEFINE_SUBSYSTEM(name)          namespace SYSTEM_LINKSPACE_TAG { SubSystemID SUBSYSTEM_NAME(name)(#name); } 
#define DECLARE_SUBSYSTEM(name)         namespace SYSTEM_LINKSPACE_TAG { extern SubSystemID SUBSYSTEM_NAME(name); }
#define USSID(name)                     *(SYSTEM_LINKSPACE_TAG::SUBSYSTEM_NAME(name))

template <typename T>
class SubSystem : public Singleton<T>
{
public:
    SubSystem() {};
    ~SubSystem() {};

public:
    virtual void RegisterDependencies(SubSystemScheduler& schedule) { /* Default No Dependency */ };

protected:
    SubSystemID m_ID;
    std::list<SubSystemID> m_Dependencies;
};

