/*
    This file is part of Ether, an open-source DirectX 12 renderer.

    Copyright (c) 2020-2022 Samuel Huang - All rights reserved.

    Ether is free software: you can redistribute it jand/or modify
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

#include "property.h"
#include "json/json.hpp"

ETH_NAMESPACE_BEGIN

class Vector2Property : public Property
{
public:
    Vector2Property(std::string name, const ethVector2* data)
        : Property(name)
        , m_Data(data)
    {
    }
    ~Vector2Property() = default;

    std::string GetData() override
    {
        nlohmann::json data;
        data["name"] = m_Name;
        data["type"] = "Vector2";
        data["values"][0] = m_Data->x;
        data["values"][1] = m_Data->y;
        return data.dump();
    }

private:
    const ethVector2* m_Data;
};

ETH_NAMESPACE_END

