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

#include "entity.h"

ETH_NAMESPACE_BEGIN

Entity::Entity(const EntityID id, const std::string& name)
    : m_ID(id)
    , m_Name(name)
    , m_Enabled(true)
{
    ETH_TOOLONLY(memset(m_Components, 0, sizeof(m_Components)));
}

Entity::~Entity()
{
}

void Entity::Serialize(OStream& ostream)
{
    Serializable::Serialize(ostream);
    ostream << m_ID;
    ostream << m_Name;
    ostream << m_Enabled;
}

void Entity::Deserialize(IStream& istream)
{
    Serializable::Deserialize(istream);
    istream >> m_ID;
    istream >> m_Name;
    istream >> m_Enabled;
}

void Entity::AddMandatoryComponents()
{
    AddComponent<TransformComponent>();
}

ETH_NAMESPACE_END

