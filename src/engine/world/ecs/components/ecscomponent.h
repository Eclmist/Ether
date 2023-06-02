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

namespace Ether::Ecs
{
template <typename T>
class EcsComponent : public Serializable
{
public:
    EcsComponent(uint32_t version, uint32_t classID);
    virtual ~EcsComponent() override = default;

public:
    static ComponentID s_ComponentID;
};

template <typename T>
Ether::Ecs::EcsComponent<T>::EcsComponent(uint32_t version, uint32_t classID)
    : Serializable(version, classID)
{
}

template <typename T>
ComponentID Ether::Ecs::EcsComponent<T>::s_ComponentID = -1;
} // namespace Ether::Ecs
