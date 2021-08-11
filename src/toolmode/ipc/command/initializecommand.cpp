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

#include "initializecommand.h"

ETH_NAMESPACE_BEGIN

InitializeCommand::InitializeCommand(const CommandData& data)
    : m_ParentWindowHandle((void*)(uint64_t)data["args"]["hwnd"]) {
}

void InitializeCommand::Execute()
{
    EngineCore::GetMainWindow().SetParentWindowHandle(m_ParentWindowHandle);
    EngineCore::GetMainWindow().Show();
    auto initResponse = std::make_shared<InitCommandResponse>(EngineCore::GetMainWindow().GetWindowHandle());
    EngineCore::GetIpcManager().QueueCommand(initResponse);
}

InitCommandResponse::InitCommandResponse(void* engineWindowHandle)
    : m_EngineWindowHandle(engineWindowHandle)
{
}

std::string InitCommandResponse::GetSendableData() const
{
    CommandData command = {
        { "command", "initialize" },
        { "args", {
            { "childhwnd", (uint64_t)m_EngineWindowHandle }
        }}
    };

    return command.dump();
}

ETH_NAMESPACE_END

