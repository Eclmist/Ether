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
    class FloatField : public Field
    {
    public:
        FloatField(const std::string& name, float& data)
            : Field(name)
            , m_Data(&data)
            , m_HasMinMax(false)
            , m_MinValue(-1)
            , m_MaxValue(-1)
        {
        }

        FloatField(const std::string& name, float& data, float minVal, float maxVal)
            : Field(name)
            , m_Data(&data)
            , m_HasMinMax(true)
            , m_MinValue(minVal)
            , m_MaxValue(maxVal)
        {
        }

        ~FloatField() = default;

        std::string GetData() override
        {
            nlohmann::json data;
            data["name"] = m_Name;
            data["type"] = "Float";
            data["values"][0] = *m_Data;

            if (m_HasMinMax)
            {
                data["properties"]["range"]["min"] = m_MinValue;
                data["properties"]["range"]["max"] = m_MaxValue;
            }

            return data.dump();
        }

        void SetData(const nlohmann::json& data) override
        {
            *m_Data = data["values"][0];
        }

    protected:
        float* const m_Data;

        const bool m_HasMinMax;
        const float m_MinValue;
        const float m_MaxValue;
    };
}

