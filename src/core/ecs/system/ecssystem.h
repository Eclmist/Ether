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

ETH_NAMESPACE_BEGIN

class EcsSystem : public NonCopyable
{
public:
    EcsSystem() = default;
    ~EcsSystem() = default;

public:
    virtual void OnEntityRegister(EntityID id) = 0;
    virtual void OnEntityDeregister(EntityID id) = 0;
    virtual void OnSceneLoad() = 0;
    virtual void OnUpdate() = 0;

public:
    inline ComponentSignature GetSignature() const { return m_Signature; }

protected:
    friend class EcsSystemManager;
    std::set<EntityID> m_MatchingEntities;
    ComponentSignature m_Signature;
};

ETH_NAMESPACE_END

