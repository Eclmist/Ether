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

#include "toolmode/pch.h"
#include "toolmode/property/field.h"

namespace Ether::Toolmode
{
    class Vector4Field : public Field
    {
    public:
        Vector4Field(const std::string& name, const ethVector4& data)
            : Field(name)
            , m_Data(&data)
        {
        }
        ~Vector4Field() = default;

        std::string GetData() override
        {
            nlohmann::json data;
            data["name"] = m_Name;
            data["type"] = "Vector4";
            data["values"][0] = m_Data->x;
            data["values"][1] = m_Data->y;
            data["values"][2] = m_Data->z;
            data["values"][3] = m_Data->w;
            return data.dump();
        }

        void SetData(const nlohmann::json& data) override
        {
            m_Data->x = data["values"][0];
            m_Data->y = data["values"][1];
            m_Data->z = data["values"][2];
            m_Data->w = data["values"][3];
        }

    private:
        ethVector4* const m_Data;
    };
}

