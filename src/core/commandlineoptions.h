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

ETH_NAMESPACE_BEGIN

class CommandLineOptions
{
public:
    CommandLineOptions();
    ~CommandLineOptions() = default;

public:
    inline bool GetUseSourceShaders() const { return m_UseSourceShaders; }
    inline HWND GetCauldronHwnd() const { return m_CauldronHwnd; }

private:
    void InitializeArg(const std::wstring& flag, const std::wstring& arg = L"");

private:
    bool m_UseSourceShaders;
    HWND m_CauldronHwnd;
};
 
ETH_NAMESPACE_END

