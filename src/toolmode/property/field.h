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
#include "parser/json/json.hpp"

ETH_NAMESPACE_BEGIN

class Field
{
public:
    Field(std::string name = "Unnamed Property")
        : m_Name(name)
    {
    }

    virtual std::string GetData() = 0;
    virtual void SetData(const nlohmann::json& data) = 0;

public:
    inline std::string GetName() const { return m_Name; }

protected:
    std::string m_Name;
};

ETH_NAMESPACE_END

