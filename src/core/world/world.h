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

#include "core/entity/entity.h"

ETH_NAMESPACE_BEGIN

class World : public NonCopyable
{
public:
    World();
    ~World() = default;

public:
    ETH_ENGINE_DLL void AddEntity(Entity* entity, Entity* parent = nullptr);
    //TODO: Remove or change to get all visuals
    std::vector<Entity*>& GetEntities() { return m_AllEntities; }

private:
    Entity m_RootEntity;
    std::vector<Entity*> m_AllEntities;
};

ETH_NAMESPACE_END
