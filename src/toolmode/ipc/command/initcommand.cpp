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

#include "initcommand.h"

ETH_NAMESPACE_BEGIN

InitCommand::InitCommand(const nlohmann::json& command)
    : m_Hwnd((void*)(uint64_t)command["args"]["hwnd"])
{
}

void InitCommand::Execute()
{
    EngineCore::SetEditorWindowHandle(m_Hwnd);
}

InitCommandResponse::InitCommandResponse(void* newWindowHandle)
    : m_Hwnd(newWindowHandle)
{
}

std::string InitCommandResponse::GetResponseData() const
{
    nlohmann::json command = {
        { "command", "initialize" },
        { "args", {
            { "childhwnd", (uint64_t)m_Hwnd }
        }}
    };

    return command.dump();
}

ETH_NAMESPACE_END

