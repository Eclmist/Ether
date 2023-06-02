/*
    This file is part of Ether, an open-source DirectX 12 renderer.

    Copyright (c) 2020-2023 Samuel Huang - All rights reserved.

    Ether is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include "pch.h"
#include "engine/world/ecs/ecstypes.h"
#include <queue>
#include <array>
#include <set>

namespace Ether::Ecs
{
class ETH_ENGINE_DLL EcsEntityManager : public Serializable
{
public:
    EcsEntityManager();
    ~EcsEntityManager() = default;

public:
    void Serialize(OStream& ostream) const override;
    void Deserialize(IStream& istream) override;

public:
    EntityID CreateEntity();
    void DestroyEntity(EntityID id);

    void SetSignature(EntityID id, EntitySignature signature);
    EntitySignature GetSignature(EntityID id);

private:
    friend class EcsManager;

private:
    std::queue<EntityID> m_AvailableEntities;
    std::array<EntitySignature, MaxNumEntities> m_EntitySignatures;
};
} // namespace Ether::Ecs
