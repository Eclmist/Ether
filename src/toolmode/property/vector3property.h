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

#include "property.h"
#include "json/json.hpp"

ETH_NAMESPACE_BEGIN

class Vector3Property : public Property
{
public:
    Vector3Property(std::string name, const ethVector3* data)
        : Property(name)
        , m_Data(data)
    {
    }
    ~Vector3Property() = default;

    std::string GetData() override
    {
        nlohmann::json data;
        data["name"] = m_Name;
        data["type"] = "Vector3";
        data["values"][0] = std::to_string(m_Data->x);
        data["values"][1] = std::to_string(m_Data->y);
        data["values"][2] = std::to_string(m_Data->z);
        return data.dump();
    }

private:
    const ethVector3* m_Data;
};

ETH_NAMESPACE_END

