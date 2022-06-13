/*
    This file is part of Ether, an open-source DirectX 12 renderer.

    Copyright (c) 2020-2022 Samuel Huang - All rights reserved.

    Ether is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#ifdef ETH_TOOLMODE
#include "toolmode/property/field.h"
#include "toolmode/property/intfield.h"
#include "toolmode/property/floatfield.h"
#include "toolmode/property/vector2field.h"
#include "toolmode/property/vector3field.h"
#include "toolmode/property/vector4field.h"
#include "toolmode/property/stringfield.h"
#endif

#ifdef ETH_TOOLMODE
// Macro instead of template because property type (Type) and variable type (typeof(var)) must be compatible
#define EDITOR_INSPECTOR_FIELD(var, name, Type) m_InspectorFields.push_back(std::make_shared<Type>(name, var))
#define EDITOR_INSPECTOR_RANGE_FIELD(var, name, Type, minVal, maxVal) m_InspectorFields.push_back(std::make_shared<Type>(name, var, minVal, maxVal))
#else
#define EDITOR_INSPECTOR_FIELD(var, name, Type) (void)0
#define EDITOR_INSPECTOR_RANGE_FIELD(var, name, Type, minVal, maxVal) (void)0
#endif


ETH_NAMESPACE_BEGIN

class Component : public Serializable
{
public:
    Component(EntityID owner);
    virtual ~Component() = default;

    void Serialize(OStream& ostream) override;
    void Deserialize(IStream& istream) override;

public:
    inline EntityID GetOwner() const { return m_Owner; }
    inline bool IsEnabled() const { return m_Enabled; }
    inline void SetEnabled(bool enabled) { m_Enabled = enabled; }

    virtual std::string GetName() const = 0;
    
#ifdef ETH_TOOLMODE
public:
    inline std::vector<std::shared_ptr<Field>> GetInspectorFields() const { return m_InspectorFields; }
protected:
    std::vector<std::shared_ptr<Field>> m_InspectorFields;
#endif

protected:
    EntityID m_Owner;
    bool m_Enabled;
};

ETH_NAMESPACE_END
