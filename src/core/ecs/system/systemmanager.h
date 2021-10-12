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

ETH_NAMESPACE_BEGIN

using SystemsArray = std::unordered_map<std::string, std::shared_ptr<System>>;

class SystemManager : public NonCopyable
{
public:
    SystemManager() = default;
    ~SystemManager() = default;

public:
    inline const SystemsArray GetSystems() const { return m_Systems; }

public:
    void InitializeSystems();
    ETH_ENGINE_DLL void AssignEntityToSystems(EntityID id, ComponentSignature signature);

private:
    template <typename T>
    void RegisterSystem()
    {
        const char* typeName = typeid(T).name();
        m_Systems.insert({ typeName, std::make_shared<T>() });
    }

private:
    SystemsArray m_Systems;
};

ETH_NAMESPACE_END

