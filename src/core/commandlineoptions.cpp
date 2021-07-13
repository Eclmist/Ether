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

#include "commandlineoptions.h"
#include <shellapi.h>
#include <iostream>

ETH_NAMESPACE_BEGIN

CommandLineOptions::CommandLineOptions()
    : m_DebugMode(false)
{
    int argc;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLine(), &argc);

    for (int i = 0; i < argc; ++i)
        InitializeArg(argv[i]);
}

void CommandLineOptions::InitializeArg(const std::wstring& arg)
{
    if (arg == L"-debug")
        m_DebugMode = true;
}

ETH_NAMESPACE_END

