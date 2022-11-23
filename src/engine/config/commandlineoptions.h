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

namespace Ether
{
    class CommandLineOptions
    {
    public:
        CommandLineOptions();
        ~CommandLineOptions() = default;

    public:
        inline bool GetUseSourceShaders() const { return m_UseSourceShaders; }
        inline bool GetUseValidationLayer() const { return m_UseValidationLayer; }

    private:
        void RegisterSingleOption(const std::string& flag, const std::string& arg = "");

    private:
        bool m_UseSourceShaders;
        bool m_UseValidationLayer;
    };
}

