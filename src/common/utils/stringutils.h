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

#include "common/common.h"

namespace Ether
{
    inline std::string ToNarrowString(const std::wstring& wide)
    {
        std::string str(wide.length(), 0);
        std::transform(wide.begin(), wide.end(), str.begin(), [](wchar_t c) {
            return (char)c;
            });
        return str;
    }

    inline std::wstring ToWideString(const std::string& narrow)
    {
        std::wstring str(narrow.length(), 0);
        std::transform(narrow.begin(), narrow.end(), str.begin(), [](char c) {
            return (wchar_t)c;
        });

        return str;
    }
}

