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

#include "engine/pch.h"
#include "engine/world/ecs/components/ecscomponent.h"

namespace Ether::Ecs
{
template <typename T>
class EcsToggleComponent : public EcsComponent<T>
{
public:
    EcsToggleComponent(uint32_t version, const char* classID);
    virtual ~EcsToggleComponent() override = default;

public:
    void Serialize(OStream& ostream) const override;
    void Deserialize(IStream& istream) override;

public:
    bool m_Enabled;
};

template <typename T>
Ether::Ecs::EcsToggleComponent<T>::EcsToggleComponent(uint32_t version, const char* classID)
    : EcsComponent<T>(version, classID)
    , m_Enabled(true)
{
}

template <typename T>
void Ether::Ecs::EcsToggleComponent<T>::Serialize(OStream& ostream) const
{
    EcsComponent<T>::Serialize(ostream);
    ostream << m_Enabled;
}

template <typename T>
void Ether::Ecs::EcsToggleComponent<T>::Deserialize(IStream& istream)
{
    EcsComponent<T>::Deserialize(istream);
    istream >> m_Enabled;
}
} // namespace Ether::Ecs
