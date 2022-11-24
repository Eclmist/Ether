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

#include "toolmode/ipc/command/initializecommand.h"
#include "toolmode/ipc/ipcmanager.h"

Ether::Toolmode::InitializeCommand::InitializeCommand(const CommandData* data)
    : m_ParentWindowHandle((void*)(uint64_t)(*data)["args"]["hwnd"]) {
}

void Ether::Toolmode::InitializeCommand::Execute()
{
    SetParentWindow(m_ParentWindowHandle);
    ShowWindow();

    auto initResponse = std::make_unique<InitCommandResponse>(GetWindowHandle());
    IpcManager::Instance().QueueOutgoingCommand(std::move(initResponse));
}

Ether::Toolmode::InitCommandResponse::InitCommandResponse(void* engineWindowHandle)
    : m_EngineWindowHandle(engineWindowHandle)
{
}

std::string Ether::Toolmode::InitCommandResponse::GetSendableData() const
{
    CommandData command = {
        { "command", "initialize" },
        { "args", {
            { "childhwnd", (uint64_t)m_EngineWindowHandle }
        }}
    };

    return command.dump();
}

