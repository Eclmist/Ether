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

#ifdef ETH_TOOLMODE
#include "toolmode/property/property.h"
#include "toolmode/property/intproperty.h"
#include "toolmode/property/floatproperty.h"
#include "toolmode/property/vector2property.h"
#include "toolmode/property/vector3property.h"
#include "toolmode/property/vector4property.h"
#include "toolmode/property/stringproperty.h"

#define EDITOR_PROPERTY(var, name, Type) m_EditorProperties.push_back(std::make_unique<Type>(name, &var))
#endif

ETH_NAMESPACE_BEGIN

class Component : public Serializable
{
public:
    Component(EntityID owner);
    virtual ~Component() = default;

    inline EntityID GetOwner() const { return m_Owner; }

    inline bool IsEnabled() const { return m_Enabled; }
    inline void SetEnabled(bool enabled) { m_Enabled = enabled; }

    virtual std::string GetName() const = 0;
    
    ETH_TOOLONLY(inline std::vector<std::shared_ptr<Property>> GetEditorProperties() const { return m_EditorProperties; })

protected:
    ETH_TOOLONLY(std::vector<std::shared_ptr<Property>> m_EditorProperties);

protected:
    EntityID m_Owner;
    bool m_Enabled;
};

ETH_NAMESPACE_END
